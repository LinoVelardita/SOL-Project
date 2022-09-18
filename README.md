# SOL-Project
Project for the course of Operating Systems.

File storage server where files are stored in main memory. The storage capacity is fixed at startup and does not dynamically change while the server is running. In order to read, write or delete files within the file storage, the client must connect to the server and use an API to be developed by the student. The server performs all operations requests from clients operating always and only in main memory and never on disk. The file storage capacity, together with other configuration parameters, is defined at the start of the server via a text configuration file. The file storage server is implemented as a single multi-threaded process that can accept connections from multiple clients. The server process must be able to adequately handle a few dozen connections simultaneous by multiple clients. Each client maintains a single connection to the server on which it sends one or more requests for files stored in the server, and obtains the responses according to the "request-response" communication protocol. One file is uniquely identified by its absolute path.

For more information read progettosol-20_21.pdf and Relazione.pdf
