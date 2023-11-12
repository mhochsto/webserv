import os
import subprocess

print("Content-type: text/html\r\n\r\n")
print("<!doctype html>")
print("<html>")
print("  <head>")
print("    <meta name ='viewport' content='width=device-width, initital-scale=1.0'>")
print("    <title>Thankyou Page</title>")
print("  </head>")
print("  <body>")
print("    <h2> Overview of all Enviroment Vars:</h2>")
result = subprocess.run("ls", shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
print(result.stdout.encode("utf-8").decode("utf-8"))
print("  </body>")
print("</html>")