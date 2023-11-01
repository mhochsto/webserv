import os

print("Content-type: text/html\r\n\r\n")
print("<!doctype html>")
print("<html>")
print("  <head>")
print("    <meta name ='viewport' content='width=device-width, initital-scale=1.0'>")
print("    <title>Thankyou Page</title>")
print("  </head>")
print("  <body>")
print("    <h2> Overview of all Enviroment Vars:</h2>")
for key, value in os.environ.items():
    print("    <p>" + key +"=" + value + "</p>")

print("  </body>")
print("</html>")