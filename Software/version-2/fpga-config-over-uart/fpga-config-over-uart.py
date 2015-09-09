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
  startAddr = ''
  binFile = ''
  try:
    opts, args = getopt.getopt(argv,"hp:a:b:",["serialPort=","startAddress=","binFile="])
  except getopt.GetoptError:
    print "Flags: -p <serialPort> -a <startAddress, example 12AB72FE> -b <binFile>"
    sys.exit(2)
  for opt, arg in opts:
    if opt == '-h':
      print "Flags: -p <serialPort> -a <startAddress, example 12AB72FE> -b <binFile>"
      sys.exit()
    elif opt in ("-p", "--serialPort"):
      serPort = arg
    elif opt in ("-a", "--startAddress"):
      startAddr = arg
    elif opt in ("-b", "--binFile"):
      binFile = arg
  serialSend(serPort, startAddr, binFile)


def convertIntToHexString(int_value):
  encoded = format(int_value, 'x')
  length = len(encoded)
  encoded = encoded.zfill(8)
  return encoded.decode('hex')

def waitForAck(serialPort):
  response = serialPort.read(1)
  if (response and ord(response) == int(0xDD)):
    print bcolors.OKGREEN + "ACK received!" + bcolors.ENDC
  else:
    print bcolors.FAIL + "\n****** ACK not received!!! ******" + bcolors.ENDC
    sys.exit()


def serialSend(serialPort, startAddress, binaryFile):
  # Make sure the startAddress is on the start of a page in flash (multiple of 256)
  if int(startAddress) % 256 != 0:
    print bcolors.FAIL + "Startaddress not valid! Must be multiple of 256" + bcolors.ENDC
    sys.exit()

  # Convert to bytearray
  startAddressAsByteArray = bytearray.fromhex(startAddress)

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

  # Set the flash write address to 0x00000000
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
  newAddressAsByteArray = bytearray(convertIntToHexString(int(startAddress) + 256))
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
