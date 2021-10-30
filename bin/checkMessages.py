#!/usr/bin/python
# Check all M_ messages are used.

import os
import re

namePattern = "^\s*const\s*char\s*(M_[A-Z]*)\[\]\s*PROGMEM"

messageFileName = "Messages.h"
fileNames = os.listdir()
fileNames.remove(messageFileName)

used = 0
unused = 0

# Process all lines in the messageFile
with open(messageFileName) as messageFile:
    for line in messageFile:
        match = re.match(namePattern, line)

        # Process all messages
        if match is not None:
            found = False
            used += 1
            messageName = match.group(1)

            # Check if messageFile itself references the message
            with open(messageFileName) as messageFile2:
                refPattern = "^.* " + messageName + "[ ,]"
                for messageLine in messageFile2:
                    matchSelf = re.match(refPattern, messageLine)
                    if matchSelf is not None:
                        found = True
                        # print("Reference", messageName, "in", messageFileName)
                        break

            # Check if any other file references the message
            for fileName in fileNames:
                if found:
                    # print("Found", messageName)
                    break

                with open(fileName) as sourceFile:
                    for sourceLine in sourceFile:
                        if messageName in sourceLine:
                            found = True
                            # print("Found", messageName, "in", fileName)
                            break

            if not found:
                print("Unused", messageName)
                unused += 1

print("Messages:", used)
if unused:
    print("Unused:", unused)
else:
    print("All used")
