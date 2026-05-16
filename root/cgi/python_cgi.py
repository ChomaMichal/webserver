#!/usr/bin/env python3
import sys

def main():
    # HTTP header
    # Simple HTML response
    print("<!doctype html>")
    print("<html><head><meta charset='utf-8'><title>CGI Test</title></head>")
    print("<body><h1>CGI for Python is working</h1></body></html>")

if __name__ == "__main__":
    main()
