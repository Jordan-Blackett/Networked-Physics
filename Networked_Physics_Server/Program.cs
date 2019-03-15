using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;
using System.Threading.Tasks.Dataflow; // BufferBlock
using System.Diagnostics; // Stopwatch
using Google.Protobuf;
using Google.Protobuf.ProtobufMessage;

namespace ConsoleApp1
{
    class Program
    {
        // State object for reading client data asynchronously  
        public class StateObject
        {
            // Client  socket.
            public Socket workSocket = null;
            // Size of receive buffer.
            public const int BufferSize = 1024;
            // Receive buffer.
            public byte[] buffer = new byte[BufferSize];
            // Received data string.
            public StringBuilder sb = new StringBuilder();
            // Client Object ID
            public int netID;
            public int clientSequence;
            public int clientBaseSequence;
            public bool host;
        }

        public static List<StateObject> clients = new List<StateObject>();
        public static int hostID = 0;

        // Game World
        public class Object
        {
            public float positionX;
            public float positionY;
            public float linear_velocityX;
            public float linear_velocityY;
            public float angle;
        }

        public class World
        {
            public bool objectInit = false;
            public List<Object> objects = new List<Object>();

            public void init(int count)
            {
                for(int i = 0; i < count; i++)
                {
                    Object item = new Object();
                    item.positionX = 100;
                    item.positionY = 100;
                    objects.Add(item);
                }
            }
        }

        public static World gameWorld = new World();

        // BufferBlock
        public static BufferBlock<BufferBlockWrapperTCP> asyncQueueTCP = new BufferBlock<BufferBlockWrapperTCP>();
        public static BufferBlock<BufferBlockWrapperUDP> asyncQueueUDP = new BufferBlock<BufferBlockWrapperUDP>();

        public class BufferBlockWrapperTCP
        {
            public TCPMessage message;
            public int netID;
        }

        public class BufferBlockWrapperUDP
        {
            public UDPMessage message;
            public int netID;
        }

        static void Main(string[] args)
        {
            // Hello World
            Thread TCPThread = new Thread(new ThreadStart(StartListening));
            TCPThread.Start();

            Thread UDPThread = new Thread(new ThreadStart(UDPReceiveMessages));
            UDPThread.Start();

            Thread TCPConsumeThread = new Thread(new ThreadStart(ConsumerTCP));
            TCPConsumeThread.Start();

            Thread UDPConsumeThread = new Thread(new ThreadStart(ConsumerUDP));
            UDPConsumeThread.Start();

            Console.WriteLine("Server Setup"); // TODO: console writeline errors ie comsumer failed
            Console.Read();
        }

        static void ConsumerTCP()
        {
            while (true)
            {
                var comsume = ConsumeAsyncTCP(asyncQueueTCP);
                comsume.Wait();
            }
        }

        static void ConsumerUDP()
        {
            while (true)
            {
                var comsume = ConsumeAsyncUDP(asyncQueueUDP);
                comsume.Wait();
            }
        }

        // NetID
        public static int netIDPrefix = 1; //0 bugs protobuf!

        // Thread signal
        public static ManualResetEvent allDone = new ManualResetEvent(false);

        #region TCP

        public static void StartListening()
        {
            // Data buffer for incoming data.  
            byte[] bytes = new Byte[1024];

            // Establish the local endpoint for the socket. - the DNS name of the computer
            IPHostEntry ipHostInfo = Dns.GetHostEntry(Dns.GetHostName());
            IPAddress ipAddress = ipHostInfo.AddressList[1];

            // Get local ip
            foreach (var ip in ipHostInfo.AddressList)
            {
                if (ip.AddressFamily == AddressFamily.InterNetwork)
                {
                    ipAddress = ip;
                }
            }

            int port = 8080;
            Console.WriteLine("IP: " + ipAddress + " Port: " + port);
            IPEndPoint localEndPoint = new IPEndPoint(ipAddress, port);

            // Create a TCP/IP socket.  
            Socket listener = new Socket(ipAddress.AddressFamily, SocketType.Stream, ProtocolType.Tcp);

            // Bind the socket to the local endpoint and listen for incoming connections.  
            try
            {
                listener.Bind(localEndPoint);
                listener.Listen(100);

                while (true)
                {
                    // Set the event to nonsignaled state.  
                    allDone.Reset();

                    // Start an asynchronous socket to listen for connections.  
                    listener.BeginAccept(new AsyncCallback(AcceptCallback), listener);

                    // Wait until a connection is made before continuing.  
                    allDone.WaitOne();
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }

            Console.WriteLine("TCP Listener Stoped");
        }

        
        public static void AcceptCallback(IAsyncResult ar)
        {
            // Signal the main thread to continue.  
            allDone.Set();

            // Get the socket that handles the client request.  
            Socket listener = (Socket)ar.AsyncState;
            Socket handler = listener.EndAccept(ar);

            // Create the state object.  
            StateObject state = new StateObject();
            state.workSocket = handler;

            state.netID = netIDPrefix;
            netIDPrefix++;
            clients.Add(state);

            if (hostID == 0)
                hostID = state.netID;
            else
            {
                // Message Host - New user
                BufferBlockWrapperTCP wrap = new BufferBlockWrapperTCP();
                wrap.message = TCPMessageString("newclient::" + state.netID);
                wrap.netID = hostID;

                asyncQueueTCP.SendAsync(wrap);
            }

            // Register client
            Console.WriteLine("[ Player " + state.netID + ": Has Connected ]");

            handler.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0,
                new AsyncCallback(ReadCallback), state);
        }

        public static void ReadCallback(IAsyncResult ar)
        {
            String content = String.Empty;

            // Retrieve the state object and the handler socket  
            // from the asynchronous state object
            StateObject state = (StateObject)ar.AsyncState;
            Socket handler = state.workSocket;

            // Read data from the client socket
            int bytesRead = 0;
            try
            {
                bytesRead = handler.EndReceive(ar);

                if (bytesRead > 0)
                {
                    // There  might be more data, so store the data received so far.  
                    state.sb.Append(Encoding.ASCII.GetString(state.buffer, 0, bytesRead));

                    byte protoSize = state.buffer[0];

                    // Check for end-of-file tag. If it is not there, read   
                    // more data.
                    content = state.sb.ToString();
                    //if (state.buffer.Length >= protoSize)
                    //if (content.IndexOf("<EOF>") > -1)
                    //{
                    // All the data has been read from the   
                    // client. Display it on the console.  
                    //Console.WriteLine("Read {0} bytes from socket. \n Data : {1}", content.Length, content);

                    // Parse Data to protobuf
                    //byte[] testData2 = Encoding.ASCII.GetBytes(content);
                    //TCPMessage msg = TCPMessage.Parser.ParseFrom(testData2);
                    //Google.Protobuf.Examples.AddressBook.UDPMessage msg;
                    //// Post the result to the message block.
                    ////state.target.SendAsync(msg);

                    //BufferBlockWrapper wrap = new BufferBlockWrapper();
                    //wrap.message = msg;
                    //wrap.netID = state.netID;
                    //asyncQueueTCP.SendAsync(wrap);

                    state.sb.Clear(); //= new StringBuilder();

                    handler.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0,
                    new AsyncCallback(ReadCallback), state);
                }
            }
            catch (Exception e)
            {
                //Console.WriteLine(e.ToString());

                // Player Disconnected
                Console.WriteLine("[ Player " + state.netID + ": Has Disconnected ]");
                // If host reset server
                if (state.netID == hostID)
                {
                    // Reset server
                    clients.Clear();
                    hostID = 0;
                }
                else
                {
                    clients.RemoveAt(state.netID - 1);
                }
            }
        }

        private static void TCPSendMessage(Socket handler, byte[] data)
        {
            // Convert the string data to byte data using ASCII encoding.  
            //byte[] byteData = Encoding.ASCII.GetBytes(data);
            byte[] byteData = data;
            // Begin sending the data to the remote device.  
            handler.BeginSend(byteData, 0, byteData.Length, 0,
                new AsyncCallback(SendCallback), handler);
        }

        private static void SendCallback(IAsyncResult ar)
        {
            try
            {
                // Retrieve the socket from the state object.  
                Socket handler = (Socket)ar.AsyncState;

                // Complete sending the data to the remote device.  
                int bytesSent = handler.EndSend(ar);
                //Console.WriteLine("Sent {0} bytes to client.", bytesSent);

                //handler.Shutdown(SocketShutdown.Both);
                //handler.Close();
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        #endregion

        #region TCP Consumer

        // TCP Consumer
        static async Task<BufferBlockWrapperTCP> ConsumeAsyncTCP(ISourceBlock<BufferBlockWrapperTCP> source)
        {
            // Read from the source buffer
            while (await source.OutputAvailableAsync())
            {
                BufferBlockWrapperTCP wrapper = await source.ReceiveAsync();

                TCPMessage msg = wrapper.message;

                ConsumeMessageTCP(msg, wrapper.netID);
            }

            return null;
        }

        static void ConsumeMessageTCP(TCPMessage msg, int netID)
        {
            char delimiterChar = ':';
            string[] messageSplit = msg.Msg.Split(delimiterChar);

            if (messageSplit[0] == "newclient")
            {
                // Send to host
                TCPSendMessageToClient(msg, hostID);
            }

            if (messageSplit[0] == "snapshotack")
            {
                TCPSendMessageToClient(msg, hostID);
            }

            //Console.Write(messageSplit[0]);
            //switch (msg.MsgCase)
            //{
            //    case TCPMessage.MsgOneofCase.M2:
            //        // Split message
            //        char delimiterChar = ':';
            //        string[] messageSplit = msg.M2.Sentmessage.Split(delimiterChar);

            //        // Get_Current_Players - Return current players in the lobby
            //        if (messageSplit[0] == "hostgame")
            //        {
            //            // Create game
            //            // TODO: Function
            //            GameSimulation game = new GameSimulation();
            //            game.Init();

            //            //game.Lobby();
            //            // TODO: place thread somewhere so i dont lose thread
            //            Thread GameLoopThread = new Thread(new ThreadStart(game.Lobby));
            //            GameLoopThread.Start();
            //            // GameLoopThread.
            //            // GameLoopThread.Join(6, true);
            //            //game.Join(netID, true);

            //            clients[netID].gameID = Games[GameIDPrefix - 1].Join(netID, true);
            //            //if()
            //            // send host::id
            //            //client id . game id = game prefix
            //            // Send data back
            //            // game id
            //            //Console.WriteLine("HOST!");
            //            //Console.WriteLine("[ NetID: " + clients[clientIndex].netID.ToString() + " Username: " + clients[clientIndex].username + " ] Connected");
            //            // Send
            //            TCPMessage serverReply = MessageM2(msg.M2.Sentmessage + "::");
            //            SendMessage(serverReply, netID);
            //        }

            //        // Join game lobby
            //        else if (messageSplit[0] == "joingame")
            //        {
            //            int gameID = Int32.Parse(messageSplit[2]);

            //            if (Games.ContainsKey(gameID))
            //            {
            //                if (Games[gameID].Join(netID, false) == 0)
            //                {
            //                    // Lobby Full
            //                    // TODO: Send error
            //                }
            //                else
            //                {
            //                    clients[netID].gameID = GameIDPrefix - 1;
            //                    Console.WriteLine(GameIDPrefix - 1);
            //                    TCPMessage serverReply = MessageM2(msg.M2.Sentmessage + "true::");
            //                    SendMessage(serverReply, netID);
            //                }
            //            }
            //            else
            //            {
            //                // TODO: Failed to join error messages eg: full or not there anymore
            //                TCPMessage serverReply = MessageM2(msg.M2.Sentmessage + "false::");
            //                SendMessage(serverReply, netID);
            //            }
            //        }

            //        // Get Games
            //        else if (messageSplit[0] == "getcurrentgames")
            //        {
            //            TCPMessage serverReply = getCurrentGames();
            //            SendMessage(serverReply, netID);
            //        }

            //        // Lobby Settings
            //        else if (messageSplit[0] == "lobbysettings")
            //        {
            //            int gameID = clients[netID].gameID;
            //            Games[gameID].SetLobbySettings(messageSplit, netID, msg);
            //        }

            //        // Lobby Settings TODO: WHAT IS THIS
            //        else if (messageSplit[0] == "ingame")
            //        {
            //            if (messageSplit[2] == "ready")
            //            {
            //                //clients[netID].gameSettings.ingame = true;

            //                bool ready = true;
            //                //check if everyone is ready
            //                foreach (KeyValuePair<int, ClientObject> entry in clients)
            //                {
            //                    // A Player isn't Ready
            //                    //if (!entry.Value.gameSettings.ingame)
            //                    //{
            //                    //   ready = false;
            //                    //}
            //                }

            //                if (ready)
            //                {
            //                    TCPMessage serverReply = MessageM2(msg.M2.Sentmessage + ready + "::");
            //                    //SendMessageToAll(serverReply);

            //                    gameStart = true;
            //                }
            //            }
            //        }


            //        // Chat System
            //        else if (messageSplit[0] == "chat")
            //        {
            //            int gameID = clients[netID].gameID;
            //            Games[gameID].Chat(netID, msg);
            //        }
            //        break;
            //}
        }

        #endregion

        #region UDP

        public class UDPState
        {
            public IPEndPoint e;
            public UdpClient u;
        }

        public static int listenPort = 8080;
        public static int sendPort = 8005;

        public static bool messageReceived = false;
        public static void UDPReceiveCallback(IAsyncResult ar)
        {
            UdpClient u = (UdpClient)((UDPState)(ar.AsyncState)).u;
            IPEndPoint e = (IPEndPoint)((UDPState)(ar.AsyncState)).e;
            Byte[] receiveBytes = u.EndReceive(ar, ref e);

            //Console.WriteLine("Bytes received: {0}", receiveBytes.Length);

            messageReceived = true;

            UDPState s = (UDPState)ar.AsyncState;

            UDPMessage msg = UDPMessage.Parser.ParseFrom(receiveBytes);

            BufferBlockWrapperUDP wrap = new BufferBlockWrapperUDP();
            wrap.message = msg;
            wrap.netID = clients.Find(c => ((IPEndPoint)c.workSocket.RemoteEndPoint).Address.Equals(e.Address)).netID;

            asyncQueueUDP.SendAsync(wrap);

            u.BeginReceive(new AsyncCallback(UDPReceiveCallback), s);
        }

        public static void UDPReceiveMessages()
        {
            // Receive a message and write it to the console.
            IPEndPoint e = new IPEndPoint(IPAddress.Any, listenPort);
            UdpClient u = new UdpClient(e);
            UDPState s = new UDPState();
            s.e = e;
            s.u = u;
            Console.WriteLine("listening for UDP messages");
            u.BeginReceive(new AsyncCallback(UDPReceiveCallback), s);

            // Do some work while we wait for a message.
            while (!messageReceived)
            {
                // Do something
            }
        }

        public static bool messageSent = false;
        public static void UDPSendCallback(IAsyncResult ar)
        {
            UdpClient u = (UdpClient)ar.AsyncState;
            //Console.WriteLine("Bytes sent: {0}", u.EndSend(ar));
            messageSent = true;
        }

        static void UDPSendMessage(string server, Byte[] seBytes)
        {
            // Create the udp socket
            UdpClient u = new UdpClient();
            u.Connect(server, sendPort);
            Byte[] sendBytes = seBytes; //Encoding.ASCII.GetBytes(message);
            // send the message
            // the destination is defined by the call to .Connect()
            u.BeginSend(sendBytes, sendBytes.Length, new AsyncCallback(UDPSendCallback), u);
            // Do some work while we wait for the send to complete. For
            // this example, we 'll just sleep
            while (!messageSent)
            {
                //Thread.Sleep(100);
            }
        }

        #endregion

        #region UDP Consumer

        // UDP Consumer
        static async Task<BufferBlockWrapperUDP> ConsumeAsyncUDP(ISourceBlock<BufferBlockWrapperUDP> source)
        {
            // Read from the source buffer
            while (await source.OutputAvailableAsync())
            {
                BufferBlockWrapperUDP wrapper = await source.ReceiveAsync();

                UDPMessage msg = wrapper.message;
                int netID = wrapper.netID;

                ConsumeMessageUDP(msg, netID);
            }

            return null;
        }

        static void ConsumeMessageUDP(UDPMessage msg, int netID)
        {
            switch (msg.MsgCase)
            {
                // Lock Step
                case UDPMessage.MsgOneofCase.Lockstepmsg:
                {
                    break;
                }
                // Snapshot
                case UDPMessage.MsgOneofCase.Snapshotmsg:
                {
                    //if (clients.Find(c => c.netID == netID).host)
                    //Console.WriteLine("id" + netID + "id2 " + hostID);
                    if (netID == hostID)
                    {
                        // Send to clients
                        UDPSendMessageToAllButHost(msg);
                    }
                    else
                    {
                        // Send to host
                        UDPSendMessageToClient(msg, hostID);
                    }

                    break;
                }
                // State
                case UDPMessage.MsgOneofCase.Statemsg:
                {
                    if (netID == hostID)
                    {
                        // Send to clients
                        UDPSendMessageToAllButHost(msg);
                    }
                    else
                    {
                        // Send to host
                        UDPSendMessageToClient(msg, hostID);
                    }

                    break;
                }
                // Client Message
                case UDPMessage.MsgOneofCase.Udpstringmsg:
                {
                    char delimiterChar = ':';
                    string[] messageSplit = msg.Udpstringmsg.Msg.Split(delimiterChar);

                    if (messageSplit[0] == "snapshotack")
                    {
                        // Send to host
                        UDPSendMessageToClient(msg, hostID);
                    }

                    break;
                }
                // Client Input Message
                case UDPMessage.MsgOneofCase.Snapshotinputmsg:
                {
                
                    // Apply NetID
                    //msg.Snapshotinputmsg.Sequence = netID;

                    // Send to host
                        UDPSendMessageToClient(msg, hostID);
                    break;
                }
                // Client Input Message
                case UDPMessage.MsgOneofCase.Stateack:
                {
                    // Apply NetID
                    //msg.Snapshotinputmsg.Sequence = netID;

                    // Send to host
                    UDPSendMessageToClient(msg, hostID);
                    break;
                }
                // State Update
                case UDPMessage.MsgOneofCase.Predictionreconciliationmsg:
                {
                    if (netID == hostID)
                    {
                        // Send to clients
                        UDPSendMessageToAllButHost(msg);
                    }
                    else
                    {
                        // Send to host
                        UDPSendMessageToClient(msg, hostID);
                    }

                    break;
                }
                // State Update
                case UDPMessage.MsgOneofCase.Predictionreconciliationinputmsg:
                {
                    // Apply NetID
                    msg.Predictionreconciliationinputmsg.Netid = netID;

                    // Send to host
                    UDPSendMessageToClient(msg, hostID);
                    break;
                }

                // State Update
                case UDPMessage.MsgOneofCase.Servertimerequestmsg:
                {
                    // Apply NetID

                    if (netID == hostID)
                    {
                        // Send to clients
                        UDPSendMessageToAllButHost(msg);
                    }
                    else
                    {
                        // Send to host
                        msg.Servertimerequestmsg.ClientID = netID;
                        UDPSendMessageToClient(msg, hostID);
                    }

                    break;
                }
            }
        }

        #endregion

        static void TCPSendMessageToClient(TCPMessage message, int netID)
        {
            TCPSendMessage(clients[netID - 1].workSocket, message.ToByteArray());
        }

        static void UDPSendMessageToClient(UDPMessage message, int netID)
        {
            UDPSendMessage(((IPEndPoint)clients[netID-1].workSocket.RemoteEndPoint).Address.ToString(), message.ToByteArray());
        }

        static void UDPSendMessageToAll(UDPMessage message)
        {
            foreach (var client in clients)
            {
                UDPSendMessage(((IPEndPoint)client.workSocket.RemoteEndPoint).Address.ToString(), message.ToByteArray());
            }
        }

        static void UDPSendMessageToAllButHost(UDPMessage message)
        {
            foreach (var client in clients)
            {
                if(client.netID != hostID)
                {
                   UDPSendMessage(((IPEndPoint)client.workSocket.RemoteEndPoint).Address.ToString(), message.ToByteArray());
                }
            }
        }

        static TCPMessage TCPMessageString(string messageString)
        {
            TCPMessage message = new TCPMessage();
            message.Msg = messageString;
            return message;
        }
    }
}
