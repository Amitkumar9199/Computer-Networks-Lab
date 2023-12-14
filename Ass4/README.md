```py
get request command for MyBrowser.c to get the file from any server in general
GET http://<serverip>/<filepath>:<port>

put request command for MyBrowser.c to put the file to any server in general
PUT http://<serverip>/<directorypath>:<port> <filename>

get request command for MyBrowser.c to get the file from MyHTTP.c server
GET http://127.0.0.1/<filepath>:8080

put request command for MyBrowser.c to put the file to MyHTTP.c server
PUT http://127.0.0.1/<directorypath>:8080 <filename>

for example, to upload some file to MyHTTP.c server's main directory, use the following command
PUT http://127.0.0.1/.:8080 <filename>

get request command for MyBrowser.c to get the 10-http.pdf from http://cse.iitkgp.ac.in/~agupta/networks/index.html (http://203.110.245.250/~agupta/networks/index.html)
GET http://203.110.245.250/~agupta/networks/index.html

get request command to get a file (ex:- lab-assign-4-power-of-threads-2023.docx.pdf) from MyHTTP.c server using Mozilla Firefox or Google Chrome
http://localhost:8080/lab-assign-4-power-of-threads-2023.docx.pdf
```
