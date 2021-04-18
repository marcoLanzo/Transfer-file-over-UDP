# Transfer-file-over-UDP

The main aim of this project is to realize and implement a Client-Server application exploiting the Berkeley’s Socket API. This app is able to transfer files between clients and server using a Connectionless network service (socket type SOCK_DGRAM, i.e. UDP as transport layer protocol) made reliable trough the realization on the applicative layer of the protocol selective repeat protocol with shipping window N.

# The software allows:

- client-server connection without authentication.
- display on the client the files available on the server (command list).
- download of a file from the server (command get).
- upload of a file on the server (command put).
- File transfer reliably.

Communication between client and server takes place via an appropriate protocol. The communication protocol provides for the exchange of two types of messages:

### - command messages: they are displayed from the client to the server for the execution of the various operations.
### - response messages: they are displayed from the server to the client in response to a command with the outcome of the operation.

# Server features:

- Sending the response message to the list command to the requesting client; the reply message contains the filelist, that is the list of file names available for sharing.
- Sending the response message to the get command containing the requested file, if any, or an appropriate error message.
- The receipt of a put message containing the file to be uploaded to the server and the sending of a response message with the outcome of the operation. 

# Client features:

- Sending the list message to request the list of available file names.
- Sending the get message to obtain a file.
- Receiving a file requested via the get message or managing any error.
- Sending the put message to upload a file to the server and receive of the reply message with the result of the operation.

# Run the Project: 
Following the steps listed below you can run the code on your device: 
• take the "IIW" compressed file and move it within your file system.
• unzip the folder.
• open the “code” folder.
• open two separate terminals and go to the right folder.
• with the first terminal, fill in the "server.c" file inside the folder "server".
• with the second terminal, fill in the "client.c" file inside the folder"Client".
• finally perform both.
