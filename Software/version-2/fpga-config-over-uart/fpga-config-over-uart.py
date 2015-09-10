# *******************************************************************************
# * @file    fpga-config-over-uart.py
# * @author  Hampus Sandberg
# * @version 0.1
# * @date    2015-09-08
# * @brief
# *******************************************************************************
#  Copyright (c) 2015 Hampus Sandberg.
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
# *******************************************************************************

import serial
from serial import SerialException
import sys
import os
import getopt
import binascii

class bcolors:
  HEADER = '\033[95m'
  OKBLUE = '\033[94m'
  OKGREEN = '\033[92m'
  WARNING = '\033[93m'
  FAIL = '\033[91m'
  ENDC = '\033[0m'
  BOLD = '\033[1m'
  UNDERLINE = '\033[4m'

def main(argv):
  serPort = ''
  bitFileNum = ''
  binFile = ''
  startConfigOfNum = ''
  try:
    opts, args = getopt.getopt(argv,"hp:n:b:s:",["serialPort=","bitFileNum=","binFile=","startConfigOfNum="])
  except getopt.GetoptError:
    print "Flags: -p <serialPort> -n <bitFileNum, (1, 2)> -b <binFile>"
    sys.exit(2)
  for opt, arg in opts:
    if opt == '-h':
      print "Flags: -p <serialPort> -n <bitFileNum, (1, 2)> -b <binFile>"
      sys.exit()
    elif opt in ("-p", "--serialPort"):
      serPort = arg
    elif opt in ("-n", "--bitFileNum"):
      bitFileNum = arg
    elif opt in ("-b", "--binFile"):
      binFile = arg
    elif opt in ("-s", "--startConfigOfNum"):
      startConfigOfNum = arg

  # Send the config file
  if (serPort != '' and bitFileNum != '' and binFile != ''):
    serialSend(serPort, bitFileNum, binFile)

  # Start the configuration
  if (serPort != '' and startConfigOfNum != ''):
    startConfig(serPort, startConfigOfNum)


# =============================================================================
# Function to convert integer to hex string
# =============================================================================
def convertIntToHexString(int_value):
  encoded = format(int_value, 'x')
  length = len(encoded)
  encoded = encoded.zfill(8)
  return encoded.decode('hex')



# =============================================================================
# Function to wait for ack
# =============================================================================
def waitForAck(serialPort):
  response = serialPort.read(1)
  if (response and ord(response) == int(0xDD)):
    print bcolors.OKGREEN + "ACK received!" + bcolors.ENDC
  else:
    print bcolors.FAIL + "\n****** ACK not received!!! ******" + bcolors.ENDC
    sys.exit()



# =============================================================================
# Function to start the config
# =============================================================================
def startConfig(serialPort, number):
  # Try to open the serial port
  try:
    ser = serial.Serial(serialPort, 115200, timeout=10)
  except:
    print bcolors.FAIL + "Invalid serial port. Is it connected?" + bcolors.ENDC
    sys.exit()

  if (not ser.isOpen()):
    print bcolors.FAIL + "Serial port not open" + bcolors.ENDC
    sys.exit()
  else:
    print bcolors.OKGREEN + "Serial port is open!" + bcolors.ENDC
    raw_input(bcolors.OKBLUE + "Press Enter to start fpga config..." + bcolors.ENDC)

  print "Sending start config of bit file number " + number + "...",
  checksum = chr(int(0xAA) ^ int(0xBB) ^ int(0xCC) ^ int(0x50) ^ int(0x01) ^ int(number))
  startConfigCommand = bytearray([0xAA, 0xBB, 0xCC, 0x50, 0x01, int(number)])
  startConfigCommand.extend(checksum)
  #print "\n" + bcolors.WARNING + binascii.hexlify(startConfigCommand) + bcolors.ENDC
  ser.write(startConfigCommand)
  # Wait for ack
  waitForAck(ser)


# =============================================================================
# Function to send the bit file
# =============================================================================
def serialSend(serialPort, bitFileNumber, binaryFile):
  # Make sure the bit file number is valid
  if int(bitFileNumber) != 1 and int(bitFileNumber) != 2:
    print bcolors.FAIL + "Bit file number can only be 1 or 2" + bcolors.ENDC
    sys.exit()
  # Calculate the start address from the bit file number
  startAddress = int(bitFileNumber) * 393216
  startAddressAsHexString = convertIntToHexString(startAddress)

  # Convert to bytearray
  startAddressAsByteArray = bytearray(startAddressAsHexString)
  #print "\n" + bcolors.WARNING + binascii.hexlify(startAddressAsByteArray) + bcolors.ENDC

  # Try to open the serial port
  try:
    ser = serial.Serial(serialPort, 115200, timeout=10)
  except:
    print bcolors.FAIL + "Invalid serial port. Is it connected?" + bcolors.ENDC
    sys.exit()

  if (not ser.isOpen()):
    print bcolors.FAIL + "Serial port not open" + bcolors.ENDC
    sys.exit()
  else:
    print bcolors.OKGREEN + "Serial port is open!" + bcolors.ENDC
    raw_input(bcolors.OKBLUE + "Press Enter to start sending data..." + bcolors.ENDC)


  # Erase any old bit files in the flash at this position
  print "Sending erase bit file command",
  checksum = chr(int(0xAA) ^ int(0xBB) ^ int(0xCC) ^ int(0x22) ^ int(0x01) ^ int(bitFileNumber))
  eraseCommand = bytearray([0xAA, 0xBB, 0xCC, 0x22, 0x01, int(bitFileNumber)])
  eraseCommand.extend(checksum)
  ser.write(eraseCommand)
  # Wait for ack
  waitForAck(ser)

  # Set the flash write address to the start address
  print "Sending write address command for info",
  checksum = chr(int(0xAA) ^ int(0xBB) ^ int(0xCC) ^ int(0x10) ^ int(0x04))
  checksum = chr(ord(checksum) ^ int(startAddressAsByteArray[0]))
  checksum = chr(ord(checksum) ^ int(startAddressAsByteArray[1]))
  checksum = chr(ord(checksum) ^ int(startAddressAsByteArray[2]))
  checksum = chr(ord(checksum) ^ int(startAddressAsByteArray[3]))
  writeAddressCommand = bytearray([0xAA, 0xBB, 0xCC, 0x10, 0x04])
  writeAddressCommand.extend(startAddressAsByteArray)
  writeAddressCommand.extend(checksum)
  ser.write(writeAddressCommand)
  # Wait for ack
  waitForAck(ser)


  # Get the filesize of the bitfile -> Number of bytes
  byteCount = os.path.getsize(binaryFile)
  # Write the filesize to the first 4 bytes of the address
  print "Will send bitfile size of " + str(byteCount) + " (" + hex(byteCount) + ") bytes...",
  msg = bytearray([0xAA, 0xBB, 0xCC, 0x30, 0x04])
  msg.extend(bytearray(convertIntToHexString(byteCount)))
  checksum = chr(int(0xAA) ^ int(0xBB) ^ int(0xCC) ^ int(0x30) ^ int(0x04))
  checksum = chr(ord(checksum) ^ msg[5] ^ msg[6] ^ msg[7] ^ msg[8])
  msg.extend(checksum)
  #print "\n" + bcolors.WARNING + binascii.hexlify(msg) + bcolors.ENDC
  ser.write(msg)
  # Wait for ack
  waitForAck(ser)


  # Move the write address forward to the next page (256 bytes forward from the start)
  newAddressAsByteArray = bytearray(convertIntToHexString(startAddress + 256))
  print "Sending write address command for data",
  checksum = chr(int(0xAA) ^ int(0xBB) ^ int(0xCC) ^ int(0x10) ^ int(0x04))
  checksum = chr(ord(checksum) ^ int(newAddressAsByteArray[0]))
  checksum = chr(ord(checksum) ^ int(newAddressAsByteArray[1]))
  checksum = chr(ord(checksum) ^ int(newAddressAsByteArray[2]))
  checksum = chr(ord(checksum) ^ int(newAddressAsByteArray[3]))
  writeAddressCommand = bytearray([0xAA, 0xBB, 0xCC, 0x10, 0x04])
  writeAddressCommand.extend(newAddressAsByteArray)
  writeAddressCommand.extend(checksum)
  ser.write(writeAddressCommand)
  # Wait for ack
  waitForAck(ser)


  # Read the bitfile
  with open(binaryFile, "rb") as f:
    byte = 1
    checksum = 0
    count = 0
    data = bytearray()
    iterationCount = 0
    while byte:
      # Do stuff with byte.
      byte = f.read(1)
      if (byte):
        data.extend(byte)
        count += 1
        if (count == 1):
          checksum = byte
        else:
          checksum = chr(ord(checksum) ^ ord(byte))

        if (count == 128):
          iterationCount += 1
          print str(iterationCount) + " : " + str(count) + "bytes read...",

          # Add the header, command and data count to the checksum
          checksum = chr(ord(checksum) ^ int(0xAA))
          checksum = chr(ord(checksum) ^ int(0xBB))
          checksum = chr(ord(checksum) ^ int(0xCC))
          checksum = chr(ord(checksum) ^ int(0x30))
          checksum = chr(ord(checksum) ^ count)
          print "checksum: 0x" + binascii.hexlify(checksum) + "...",

          # Construct the message
          msg = bytearray([0xAA, 0xBB, 0xCC, 0x30, count])
          msg.extend(data)
          msg.extend(checksum)
          # DEBUG: Print the message
          #print "\n" + bcolors.WARNING + binascii.hexlify(msg) + bcolors.ENDC
          #raw_input(bcolors.OKBLUE + "Stop a bit" + bcolors.ENDC)

          # Send the message
          print "sending data...",
          ser.write(msg)

          # Wait for ack
          waitForAck(ser)

          # Clear the data that has been sent
          del data[0:count]
          count = 0

    # Send the last bytes if there are any
    if (count != 0):
      print str(count) + " bytes left to send...",
      # Add the header, command and data count to the checksum
      checksum = chr(ord(checksum) ^ int(0xAA))
      checksum = chr(ord(checksum) ^ int(0xBB))
      checksum = chr(ord(checksum) ^ int(0xCC))
      checksum = chr(ord(checksum) ^ int(0x30))
      checksum = chr(ord(checksum) ^ count)
      print "checksum: 0x" + binascii.hexlify(checksum) + "...",

      # Construct the message
      msg = bytearray([0xAA, 0xBB, 0xCC, 0x30, count])
      msg.extend(data)
      msg.extend(checksum)
      # DEBUG: Print the message
      #print "\n" + bcolors.WARNING + binascii.hexlify(msg) + bcolors.ENDC

      # Send the message
      print "sending data...",
      ser.write(msg)

      # Wait for ack
      waitForAck(ser)

    print bcolors.OKGREEN + "Done sending " + str(byteCount) + " bytes of data" + bcolors.ENDC


if __name__ == "__main__":
  main(sys.argv[1:])
