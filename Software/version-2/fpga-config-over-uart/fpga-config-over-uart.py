# ******************************************************************************
# * @file    fpga-config-over-uart.py
# * @author  Hampus Sandberg
# * @version 0.2
# * @date    2016-09-04
# * @brief
# ******************************************************************************
#  Copyright (c) 2016 Hampus Sandberg.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
# ******************************************************************************

import serial
from serial import SerialException
import sys
import os
import glob
import time
import getopt
import binascii
import datetime
import hashlib
from progressbar import ProgressBar, Bar, Percentage

# Colorama used for colored terminal text
# Fore: BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE, RESET.
# Back: BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE, RESET.
# Style: DIM, NORMAL, BRIGHT, RESET_ALL
from colorama import init
from colorama import Fore, Back, Style
init()

verboseMode = 0
activeSerialPort = 0

def main(argv):
  serialPort = ''
  bitFileNumber = ''
  binaryFile = ''

  shouldStoreBitfile = 0
  shouldDeleteBitfile = 0
  shouldConfigBitfile = 0
  shouldReadHeaders = 0

  try:
    opts, args = getopt.getopt(argv, "hlp:n:b:", ["help", "store", "delete", "config", "read", "verbose"])
  except getopt.GetoptError as err:
    print Fore.RED + "ERROR: " + str(err) + Fore.RESET
    showUsage(sys.argv[0])
    sys.exit(1)

  # Loop through all arguments
  for opt, arg in opts:
    # --------------------------------------------------------------------------
    # Help
    if opt in ("-h", "--help"):
      showUsage(sys.argv[0])
      sys.exit(0)
    # --------------------------------------------------------------------------
    # List serial ports
    elif opt in "-l":
      print Fore.CYAN + "INFO: Here are the available serial ports:" + Fore.GREEN
      print(serial_ports())
      print Fore.RESET
      sys.exit(1)
    # --------------------------------------------------------------------------
    # Set the serial port to use
    elif opt in "-p":
      serialPort = arg
    # --------------------------------------------------------------------------
    # Bit file number to save to
    elif opt in "-n":
      bitFileNumber = int(arg)
    # --------------------------------------------------------------------------
    # Path to bitfile
    elif opt in "-b":
      binaryFile = arg
    # --------------------------------------------------------------------------
    # Store bitfile at position
    elif opt in "--store":
      shouldStoreBitfile = 1
    # --------------------------------------------------------------------------
    # Delete bitfile at position
    elif opt in "--delete":
      shouldDeleteBitfile = 1
    # --------------------------------------------------------------------------
    # Configure FPGA with bitfile
    elif opt in "--config":
      shouldConfigBitfile = 1
    # --------------------------------------------------------------------------
    # Read bitfile header
    elif opt in "--read":
      shouldReadHeaders = 1
    # --------------------------------------------------------------------------
    # Verbose mode
    elif opt in "--verbose":
      global verboseMode
      verboseMode = 1
    # --------------------------------------------------------------------------


  # ****************************************************************************
  # Check if a serial port was defined
  if (serialPort == ''):
    print Fore.RED + "ERROR: No serial port defined, (use -p)" + Fore.RESET
    sys.exit(1)

  # ----------------------------------------------------------------------------
  # Read bit file headers
  if (shouldReadHeaders == 1 and serialPort != ''):
    readHeaders(serialPort)
    sys.exit(0)

  # ****************************************************************************
  # Check if a bitfile number was defined
  if (bitFileNumber == ''):
    print Fore.RED + "ERROR: No bitfile number defined, (use -n)" + Fore.RESET
    sys.exit(1)

  # ----------------------------------------------------------------------------
  # Store the bitfile
  if (shouldStoreBitfile == 1 and binaryFile != ''):
    storeBitfile(serialPort, bitFileNumber, binaryFile)
    sys.exit(0)
  elif (shouldStoreBitfile == 1):
    print binaryFile
    print Fore.RED + "ERROR: No bitfile path defined, (use -b)" + Fore.RESET
    sys.exit(1)

  # ----------------------------------------------------------------------------
  # Delete the bitfile
  if (shouldDeleteBitfile == 1):
    deleteBitfile(serialPort, bitFileNumber)
    sys.exit(0)

  # ----------------------------------------------------------------------------
  # Start the configuration
  if (shouldConfigBitfile == 1):
    startConfig(serialPort, bitFileNumber)
    sys.exit(0)

  # ****************************************************************************
  print Fore.RED + "ERROR: Something went wrong, check the arguments" + Fore.RESET
  showUsage(sys.argv[0])
  sys.exit(1)
  # ****************************************************************************


# ==============================================================================
# Function to show how to use the software
# ==============================================================================
def showUsage(name):
  print Fore.CYAN + "usage:"
  print "  python " + name + " [-h, --help] [-l] [-p] [-n] [-b] [--store] [--delete] [--config] [--read] [-v]"
  print "options:"
  print "  -h, --help : Display this help"
  print "  -l         : List the available serial ports"
  print "  -p arg     : Specifiy the serial port to use"
  print "  -n arg     : Specifiy the bitfile number"
  print "  -b arg     : Path to the bitfile"
  print "  --store    : Store the specified bitfile"
  print "  --delete   : Delete the specified bitfile"
  print "  --config   : Start config of the specified bitfile"
  print "  --read     : Read the bitfile headers stored in flash"
  print "  --verbose  : Verbose mode, i.e. display all information"
  print ""
  print "examples:"
  print "  List all available serial ports:"
  print "    python " + name + " -l"
  print "  Read bitfile headers:"
  print "    python " + name + " -p /dev/ttyS0 --read"
  print "  Store a bitfile named example.rbf at position 2:"
  print "    python " + name + " -p /dev/ttyS0 --store -n 2 -b /path/to/example.rbf"
  print "  Delete the bitfile at position 4:"
  print "    python " + name + " -p /dev/ttyS0 --delete -n 4"
  print Fore.RESET

# ==============================================================================
# Function to get the available serial ports
# :raises EnvironmentError:
#   On unsupported or unknown platforms
# :returns:
#   A list of the serial ports available on the system
# Source: http://stackoverflow.com/questions/12090503/listing-available-com-ports-with-python
# ==============================================================================
def serial_ports():
  if sys.platform.startswith('win'):
    ports = ['COM%s' % (i + 1) for i in range(256)]
  elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
    # this excludes your current terminal "/dev/tty"
    ports = glob.glob('/dev/tty[A-Za-z]*')
  elif sys.platform.startswith('darwin'):
    ports = glob.glob('/dev/tty.*')
  else:
    raise EnvironmentError('Unsupported platform')

  result = []
  for port in ports:
    try:
      s = serial.Serial(port)
      s.close()
      result.append(port)
    except (OSError, serial.SerialException):
      pass
  return result

# ==============================================================================
# Function to open a serial port
# ==============================================================================
def openSerialPort(serialPort):
  # Try to open the serial port
  global activeSerialPort
  try:
    activeSerialPort = serial.Serial(serialPort, 115200, timeout=10)
  except:
    print Fore.RED + "ERROR: Invalid serial port. Is it connected?" + Fore.RESET
    sys.exit(1)

  if (not activeSerialPort.isOpen()):
    print Fore.RED + "ERROR: Serial port not open" + Fore.RESET
    sys.exit(1)
  else:
    print Fore.CYAN + "INFO: Serial port is open" + Fore.RESET

# ==============================================================================
# Function to convert integer to hex string
# ==============================================================================
def convertIntToHexString(int_value):
  encoded = format(int_value, 'x')
  length = len(encoded)
  encoded = encoded.zfill(8)
  return encoded.decode('hex')

# ==============================================================================
# Function to extend a message with the checksum
# ==============================================================================
def extendMessageWithChecksum(message):
  checksum = 0
  for n in message:
    checksum = checksum ^ n
  message.extend(chr(checksum))
  return message

# ==============================================================================
# Function to calculate the md5 checksum of a file
# ==============================================================================
def md5Checksum(filePath):
  with open(filePath, 'rb') as fileHandle:
    m = hashlib.md5()
    while True:
      data = fileHandle.read(8192)
      if not data:
        break
      m.update(data)
    return m.hexdigest()

# ==============================================================================
# Function to wait for ack
# ==============================================================================
def waitForAck(serialPort):
  response = serialPort.read(1)
  if (response and ord(response) == int(0xDD)):
    if (verboseMode == 1):
      print Fore.GREEN + "INFO: ACK received!" + Fore.RESET
  else:
    print Fore.RED + "\nERROR: ****** ACK not received!!! ******" + Fore.RESET
    sys.exit(1)


# ==============================================================================
# Function to store the bitfile
# ==============================================================================
def storeBitfile(serialPort, bitFileNumber, binaryFile):
  print Fore.CYAN + "INFO: Store bitfile function" + Fore.RESET

  # Make sure the bit file number is valid
  if (bitFileNumber == 0 or bitFileNumber > 5):
    print Fore.RED + "ERROR: Bit file number can only be 1 to 5" + Fore.RESET
    sys.exit(1)
  # Calculate the start address from the bit file number
  startAddress = bitFileNumber * 393216
  startAddressAsHexString = convertIntToHexString(startAddress)

  # Convert to bytearray
  startAddressAsByteArray = bytearray(startAddressAsHexString)
  #print "\n" + Fore.YELLOW + binascii.hexlify(startAddressAsByteArray) + Fore.RESET

  # Try to open the serial port
  openSerialPort(serialPort)
  global activeSerialPort
  raw_input(Fore.YELLOW + "ACTION: Press Enter to start sending data..." + Fore.RESET)

  # ----------------------------------------------------------------------------
  # Erase any old bit files in the flash at this position
  if (verboseMode == 1):
    print "INFO: Sending erase bit file command",
  eraseCommand = bytearray([0xAA, 0xBB, 0xCC, 0x22, 0x00, 0x01, bitFileNumber])
  eraseCommand = extendMessageWithChecksum(eraseCommand)
  activeSerialPort.write(eraseCommand)
  # Wait for ack
  waitForAck(activeSerialPort)

  # ----------------------------------------------------------------------------
  # Set the flash write address to the start address
  if (verboseMode == 1):
    print "INFO: Sending write address command for info",
  writeAddressCommand = bytearray([0xAA, 0xBB, 0xCC, 0x10, 0x00, 0x04])
  writeAddressCommand.extend(startAddressAsByteArray)
  writeAddressCommand = extendMessageWithChecksum(writeAddressCommand)
  activeSerialPort.write(writeAddressCommand)
  # Wait for ack
  waitForAck(activeSerialPort)
  
  # ----------------------------------------------------------------------------
  # Get the filesize of the bitfile -> Number of bytes
  byteCount = os.path.getsize(binaryFile)
  # Write the filesize to the first 4 bytes of the address
  if (verboseMode == 1):
    print "INFO: Will send bitfile size of " + str(byteCount) + " (" + hex(byteCount) + ") bytes...",
  msg = bytearray([0xAA, 0xBB, 0xCC, 0x30, 0x00, 0x04])
  msg.extend(bytearray(convertIntToHexString(byteCount)))
  msg = extendMessageWithChecksum(msg)
  #print "\n" + Fore.YELLOW + binascii.hexlify(msg) + Fore.RESET
  # Send the message
  activeSerialPort.write(msg)
  # Wait for ack
  waitForAck(activeSerialPort)

  # ----------------------------------------------------------------------------
  # Write the next 64 bytes with the name of the bitfile
  fileName = os.path.basename(binaryFile)
  # Cut the string if it's too long
  if len(fileName) > 64:
    fileName = fileName[:64]
  # Fill the string with spaces if it's too short
  elif len(fileName) < 64:
    fileName = fileName.ljust(64)
  # Build the message
  msg = bytearray([0xAA, 0xBB, 0xCC, 0x30, 0x00, 0x40])
  msg.extend(bytearray(fileName))
  msg = extendMessageWithChecksum(msg)
  # Send the message
  activeSerialPort.write(msg)
  # Wait for ack
  waitForAck(activeSerialPort)

  # ----------------------------------------------------------------------------
  # # Write the next 6 bytes with the current date and time
  # # Format: YYMMDDHHMMSS
  # now = datetime.datetime.now()
  # msg = bytearray([0xAA, 0xBB, 0xCC, 0x30, 0x00, 0x06, now.year-2000, now.month, now.day, now.hour, now.minute, now.second])
  # msg = extendMessageWithChecksum(msg)
  # # Send the message
  # activeSerialPort.write(msg)
  # # Wait for ack
  # waitForAck(activeSerialPort)

  # Write the next 6 bytes with the modified date and time of the bitfile
  # Format: YYMMDDHHMMSS
  modifiedTime = os.path.getmtime(binaryFile)
  modTime = datetime.datetime.fromtimestamp(modifiedTime)
  msg = bytearray([0xAA, 0xBB, 0xCC, 0x30, 0x00, 0x06, modTime.year-2000, modTime.month, modTime.day, modTime.hour, modTime.minute, modTime.second])
  msg = extendMessageWithChecksum(msg)
  # Send the message
  activeSerialPort.write(msg)
  # Wait for ack
  waitForAck(activeSerialPort)

  # ----------------------------------------------------------------------------
  # # Write the next 16 bytes with the md5 checksum of the bitfile
  fileChecksum = md5Checksum(binaryFile).decode("hex")
  msg = bytearray([0xAA, 0xBB, 0xCC, 0x30, 0x00, 0x10])
  msg.extend(bytearray(fileChecksum))
  msg = extendMessageWithChecksum(msg)
  # Send the message
  activeSerialPort.write(msg)
  # Wait for ack
  waitForAck(activeSerialPort)

  # ----------------------------------------------------------------------------
  # Move the write address forward to the next page (256 bytes forward from the start)
  newAddressAsByteArray = bytearray(convertIntToHexString(startAddress + 256))
  if (verboseMode == 1):
    print "INFO: Sending write address command for data",
  writeAddressCommand = bytearray([0xAA, 0xBB, 0xCC, 0x10, 0x00, 0x04])
  writeAddressCommand.extend(newAddressAsByteArray)
  writeAddressCommand = extendMessageWithChecksum(writeAddressCommand)
  activeSerialPort.write(writeAddressCommand)
  # Wait for ack
  waitForAck(activeSerialPort)

  # ----------------------------------------------------------------------------
  # Variables used to display a progress bar of the transmitted data
  bytesSent = 0
  pbar = ProgressBar(widgets=[Percentage(), Bar()], maxval=byteCount).start()
  if (verboseMode == 0):
    print Fore.CYAN + "INFO: Sending data:"

  # Read the bitfile
  with open(binaryFile, "rb") as f:
    byte = 1
    checksum = 0
    count = 0
    data = bytearray()
    iterationCount = 0
    while byte:
      # Progress bar
      if (verboseMode == 0):
        pbar.update(bytesSent)

      # Do stuff with byte.
      byte = f.read(1)
      if (byte):
        data.extend(byte)
        count += 1

        if (count == 256):
          bytesSent += 256.0
          iterationCount += 1
          if (verboseMode == 1):
            print str(iterationCount) + " : " + str(count) + "bytes read...",

          # Add the header, command and data count to the checksum
          # Construct the message
          msg = bytearray([0xAA, 0xBB, 0xCC, 0x30, 0x01, 0x00])
          msg.extend(data)
          msg = extendMessageWithChecksum(msg)
          # if (verboseMode == 1):
            # print "checksum: 0x" + binascii.hexlify(msg[len(msg)]) + "...",
          # DEBUG: Print the message
          #print "\n" + Fore.YELLOW + binascii.hexlify(msg) + Fore.RESET
          #raw_input(Fore.YELLOW + "Stop a bit" + Fore.RESET)

          # Send the message
          if (verboseMode == 1):
            print "sending data...",
          activeSerialPort.write(msg)

          # Wait for ack
          waitForAck(activeSerialPort)

          # Clear the data that has been sent
          del data[0:count]
          count = 0

    # Send the last bytes if there are any
    if (count != 0):
      if (verboseMode == 1):
        print str(count) + " bytes left to send...",
      # Progress bar
      elif (verboseMode == 0):
        pbar.update(bytesSent)

      # Construct the message
      msg = bytearray([0xAA, 0xBB, 0xCC, 0x30, 0x00, count])
      msg.extend(data)
      msg = extendMessageWithChecksum(msg)
      # if (verboseMode == 1):
        # print "checksum: 0x" + binascii.hexlify(msg[len(msg)]) + "...",
      # DEBUG: Print the message
      #print "\n" + Fore.YELLOW + binascii.hexlify(msg) + Fore.RESET

      # Send the message
      if (verboseMode == 1):
        print "sending data...",
      activeSerialPort.write(msg)

      # Wait for ack
      waitForAck(activeSerialPort)

    if (verboseMode == 0):
      pbar.finish()
    print "INFO: Done sending " + str(byteCount) + " bytes of data" + Fore.RESET

# ==============================================================================
# Function to deleta the bitfile
# ==============================================================================
def deleteBitfile(serialPort, bitFileNumber):
  print Fore.CYAN + "INFO: Delete bitfile function" + Fore.RESET

  # Make sure the bit file number is valid
  if (bitFileNumber == 0 or bitFileNumber > 5):
    print Fore.RED + "ERROR: Bit file number can only be 1 to 5" + Fore.RESET
    sys.exit(1)
  # Calculate the start address from the bit file number
  startAddress = bitFileNumber * 393216
  startAddressAsHexString = convertIntToHexString(startAddress)

  # Convert to bytearray
  startAddressAsByteArray = bytearray(startAddressAsHexString)
  #print "\n" + Fore.YELLOW + binascii.hexlify(startAddressAsByteArray) + Fore.RESET

  # Try to open the serial port
  openSerialPort(serialPort)
  global activeSerialPort
  raw_input(Fore.RED + "WARNING: Deleting a bitfile is permanent. Press enter to continue." + Fore.RESET)
  raw_input(Fore.YELLOW + "ACTION: Press Enter to start deleting bitfile number " + str(bitFileNumber) + "..." + Fore.RESET)

  # ----------------------------------------------------------------------------
  # Erase any old bit files in the flash at this position
  print Fore.CYAN + "INFO: Sending erase bitfile command" + Fore.RESET
  eraseCommand = bytearray([0xAA, 0xBB, 0xCC, 0x22, 0x00, 0x01, bitFileNumber])
  eraseCommand = extendMessageWithChecksum(eraseCommand)
  activeSerialPort.write(eraseCommand)
  # Wait for ack
  waitForAck(activeSerialPort)
  print Fore.CYAN + "INFO: Done deleting bitfile" + Fore.RESET

# ==============================================================================
# Function to start the config
# ==============================================================================
def startConfig(serialPort, bitFileNumber):
  print Fore.CYAN + "INFO: Start configuration function" + Fore.RESET

  # Try to open the serial port
  openSerialPort(serialPort)
  global activeSerialPort
  raw_input(Fore.YELLOW + "Press Enter to start FPGA config..." + Fore.RESET)

  print Fore.CYAN + "INFO: Sending start config of bitfile number " + str(bitFileNumber) + "..." + Fore.RESET
  startConfigCommand = bytearray([0xAA, 0xBB, 0xCC, 0x50, 0x00, 0x01, bitFileNumber])
  startConfigCommand = extendMessageWithChecksum(startConfigCommand)
  #print "\n" + Fore.YELLOW + binascii.hexlify(startConfigCommand) + Fore.RESET
  activeSerialPort.write(startConfigCommand)
  # Wait for ack
  waitForAck(activeSerialPort)
  print Fore.CYAN + "INFO: Done configuring bit file" + Fore.RESET

# ==============================================================================
# Function to read the bit file headers in the flash
# ==============================================================================
def readHeaders(serialPort):
  print Fore.CYAN + "INFO: Read headers function" + Fore.RESET

  # Try to open the serial port
  openSerialPort(serialPort)
  global activeSerialPort

  print Fore.CYAN + "-------------------- Format --------------------"
  print "Date and time: YY/MM/DD - HH:MM:SS"

  # Read all bitfile headers
  for currentBitFileNum in range(1, 6): 
    # Construct the message
    readHeaderCommand = bytearray([0xAA, 0xBB, 0xCC, 0x40, 0x00, 0x05, 0x00, currentBitFileNum*6, 0x00, 0x00, 90])
    readHeaderCommand = extendMessageWithChecksum(readHeaderCommand)
    activeSerialPort.write(readHeaderCommand)
    
    # Wait for the data to arrive, (4+64+6+16=90) data + 1 checksum
    while (activeSerialPort.inWaiting() < 91):
      time.sleep(0.1)

    # Bitfile size
    size1 = activeSerialPort.read(1);
    size2 = activeSerialPort.read(1);
    size3 = activeSerialPort.read(1);
    size4 = activeSerialPort.read(1);
    sizeOfBitFile1 = size1 + size2 + size3 + size4;
    sizeOfBitFile1 = int(sizeOfBitFile1.encode('hex'), 16)

    # Bitfilename
    fileName = ''
    for n in range(0, 64):
      fileName = fileName + activeSerialPort.read(1);

    # Date and time
    year    = int(activeSerialPort.read(1).encode('hex'), 16)
    month   = int(activeSerialPort.read(1).encode('hex'), 16)
    day     = int(activeSerialPort.read(1).encode('hex'), 16)
    hour    = int(activeSerialPort.read(1).encode('hex'), 16)
    minute  = int(activeSerialPort.read(1).encode('hex'), 16)
    second  = int(activeSerialPort.read(1).encode('hex'), 16)

    # Checksum for the bitfile
    fileChecksum = ''
    for n in range(0, 16):
      fileChecksum = fileChecksum + activeSerialPort.read(1);
    fileChecksum = fileChecksum.encode("hex")

    # Checksum for the message
    checksum = activeSerialPort.read(1);

    # Printout
    print Fore.CYAN + "--------------- Bitfile number " + str(currentBitFileNum) + " ---------------"
    if (sizeOfBitFile1 == 4294967295):
      print Fore.CYAN + "Bitfile is " + Fore.RED + "ERASED" + Fore.RESET
    else:
      print Fore.CYAN + "Filename:      " + Fore.GREEN + fileName + Fore.RESET
      print Fore.CYAN + "Size:          " + Fore.GREEN + str(sizeOfBitFile1) + " bytes" + Fore.RESET
      print Fore.CYAN + "Date and time: " + Fore.GREEN + str(year).zfill(2) + "/"  \
            + str(month).zfill(2) + "/" + str(day).zfill(2) + " - "         \
            + str(hour).zfill(2) + ":" + str(minute).zfill(2) + ":"         \
            + str(second).zfill(2) + Fore.RESET
      print Fore.CYAN + "MD5 Checksum:  " + Fore.GREEN + fileChecksum + Fore.RESET

  # End with a line
  print Fore.CYAN + "------------------------------------------------" + Fore.RESET

# ==============================================================================
# Main function call
# ==============================================================================
if __name__ == "__main__":
  main(sys.argv[1:])
