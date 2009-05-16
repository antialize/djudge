from sys import stdin,stdout
stdout.write(stdin.read().replace("\\","\\\\").replace("\n","\\n"))

