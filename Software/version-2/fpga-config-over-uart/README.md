# fpga-config-over-uart

### Project info
Python script that is used to transfer bit files that are used to configure an FPGA

### Dependencies
- [pyserial](https://pypi.python.org/pypi/pyserial)
- [progressbar](https://pypi.python.org/pypi/progressbar)

### Mac install instructions
1. Open terminal and make sure python is installed by typing: python
2. Install pyserial  
  Type: pip install pyserial
3. Install progressbar:  
  Unpack "progressbar-2.3.tar.gz"  
  cd into the unpacked folder  
  Type: python setup.py install

### Windows install instructions
1. Install Git for windows (to get git bash)  
  IMPORTANT: Select "Use Windows' default console window". The other alternative does not work as wanted
2. Install python-2.7.10.msi  
  NOTE: Enable the option to set the PATH, otherwise you have to add "C:\Python27\;C:\Python27\Scripts\" manually to the PATH
3. Verify installation:  
  Right click in any folder and click on "Git Bash Here"  
  Type: python -i  
  It should open a python shell with the version number set to 2.7.10  
  Type: exit()  
4. Install pyserial:  
  Type in git bash: pip install pyserial  
5. Install progressbar:  
  Unpack "progressbar-2.3.tar.gz"  
  Start a git bash in the unpacked folder  
  Type: python setup.py install  
  Done, close the bash and remove the unpacked folder  
5. Test fpga-config-over-uart:  
  Open a git bash in the fpga-config-over-uart folder  
  Type: python fpga-config-over-uart.py -h  
    This should display the help if evertyhing is working  
  Type: python fpga-config-over-uart.py -l  
    This should display the available serial ports. If you get an error make sure pyserial was installed from before.  
