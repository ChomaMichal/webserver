# Funcionality of server
- parse a config file lets make it NGINX
- implement http 1.0 at least
- need to work with web browser of choice ?chromium?
- have CGI compatibility
- upload files
- serve a static website
- default error pages
- at leas methods GET POST DELETE

## networking library
- using sockets to comunicate with clients
- - Michal's job
- - currently we have classes listen with which we can get new clients 
and stream which reads to buffer and now we are able to parse  might need modification based on needs of the rest of the project

## parsing config file
* todo
* * read up on nginx 
* * parse the file and set up the behaviour of the server

## CGI
- implement a way to fork and execve a script to have CGI compatibility
- - it is just a script which we pipe info to and than read and send it back
- - do bonus here we can do multiple languages for CGI

## http parsing
- we need to decide on http version
- - be able to parse messages when we don't have the whole messages TCP works as a stream not as a message so in 1 read we can have multiple requests at the same part



