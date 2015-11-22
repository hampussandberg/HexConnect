-- Copyright (C) 1991-2014 Altera Corporation. All rights reserved.
-- Your use of Altera Corporation's design tools, logic functions 
-- and other software and tools, and its AMPP partner logic 
-- functions, and any output files from any of the foregoing 
-- (including device programming or simulation files), and any 
-- associated documentation or information are expressly subject 
-- to the terms and conditions of the Altera Program License 
-- Subscription Agreement, the Altera Quartus II License Agreement,
-- the Altera MegaCore Function License Agreement, or other 
-- applicable license agreement, including, without limitation, 
-- that your use is for the sole purpose of programming logic 
-- devices manufactured by Altera and sold by Altera or its 
-- authorized distributors.  Please refer to the applicable 
-- agreement for further details.

-- PROGRAM    "Quartus II 64-Bit"
-- VERSION    "Version 14.1.0 Build 186 12/03/2014 SJ Web Edition"
-- CREATED    "Sat Oct 03 19:15:02 2015"

LIBRARY ieee;
USE ieee.std_logic_1164.all; 

LIBRARY work;

ENTITY communication_controller_tb IS
END communication_controller_tb;

ARCHITECTURE bdf_type OF communication_controller_tb IS 

COMPONENT communication_data_manager
  PORT(
    clk                   : in std_logic;
    reset_n               : in std_logic;
    channel_id_1          : in std_logic_vector(4 downto 0);
    channel_id_2          : in std_logic_vector(4 downto 0);
    channel_id_3          : in std_logic_vector(4 downto 0);
    channel_id_4          : in std_logic_vector(4 downto 0);
    channel_id_5          : in std_logic_vector(4 downto 0);
    channel_id_6          : in std_logic_vector(4 downto 0);
    channel_id_update     : out std_logic_vector(5 downto 0);
    channel_power         : out std_logic_vector(5 downto 0);
    channel_pin_c_output  : out std_logic_vector(5 downto 0);
    channel_direction_a   : out std_logic_vector(5 downto 0);
    channel_direction_b   : out std_logic_vector(5 downto 0);
    channel_termination   : out std_logic_vector(5 downto 0);
    rx_data_ready         : in  std_logic;
    rx_data               : in  std_logic_vector(7 downto 0);
    load_tx_data_ready    : in  std_logic;
    load_tx_data          : out std_logic;
    tx_data               : out std_logic_vector(7 downto 0);
    transfer_in_progress  : in  std_logic;
    debug_leds            : out std_logic_vector(7 downto 0)
  );
END COMPONENT;

COMPONENT spi_slave_controller
  PORT(
    clk : in std_logic;
    reset_n : IN std_logic;
    load_tx_data : IN std_logic;
    spi_mosi : IN std_logic;
    spi_cs_n : IN std_logic;
    spi_sclk : IN std_logic;
    tx_data : IN std_logic_vector(7 downto 0);
    transfer_in_progress : OUT std_logic;
    load_tx_data_ready : OUT std_logic;
    rx_data_ready : OUT std_logic;
    spi_miso : OUT std_logic;
    rx_data : OUT std_logic_vector(7 downto 0)
  );
END COMPONENT;

signal rx_data_ready_tb : std_logic;
signal load_tx_data_ready_tb : std_logic;
signal transfer_in_progress_tb : std_logic;
signal rx_data_tb : std_logic_vector(7 downto 0);
signal load_tx_data_tb : std_logic;
signal tx_data_tb : std_logic_vector(7 downto 0);


-- TB Signals
signal clk : std_logic;
signal reset_n : std_logic;

signal spi_data_ncs : std_logic;
signal spi_data_sclk : std_logic;
signal spi_data_mosi : std_logic;
signal spi_data_miso :  std_logic;

signal channel_id_1 : std_logic_vector(4 downto 0);
signal channel_id_2 : std_logic_vector(4 downto 0);
signal channel_id_3 : std_logic_vector(4 downto 0);
signal channel_id_4 : std_logic_vector(4 downto 0);
signal channel_id_5 : std_logic_vector(4 downto 0);
signal channel_id_6 : std_logic_vector(4 downto 0);
signal channel_id_update : std_logic_vector(5 downto 0);


signal debug_spi_data_ncs : std_logic;
signal debug_leds : std_logic_vector(7 downto 0);

signal channel_pin_c_output : std_logic_vector(5 downto 0);
signal channel_power : std_logic_vector(5 downto 0);


signal spi_sclk_enabled : std_logic;
signal spi_byte_to_send : std_logic_vector(7 downto 0);

begin 



comm_data_manager_instance : communication_data_manager
PORT MAP (
  clk       => clk,
  reset_n => reset_n,
  rx_data_ready => rx_data_ready_tb,
  load_tx_data_ready    => load_tx_data_ready_tb,
  transfer_in_progress  => transfer_in_progress_tb,
  channel_id_1          => channel_id_1,
  channel_id_2          => channel_id_2,
  channel_id_3          => channel_id_3,
  channel_id_4          => channel_id_4,
  channel_id_5          => channel_id_5,
  channel_id_6          => channel_id_6,
  channel_id_update     => channel_id_update,
  channel_power         => channel_power,
  channel_pin_c_output  => channel_pin_c_output,

  rx_data               => rx_data_tb,
  load_tx_data          => load_tx_data_tb,
  debug_leds => debug_leds,
  tx_data => tx_data_tb);


spi_slave_instance : spi_slave_controller
PORT MAP (
  clk => clk,
  reset_n => reset_n,
  load_tx_data => load_tx_data_tb,
  spi_mosi => spi_data_mosi,
  spi_cs_n => spi_data_ncs,
  spi_sclk => spi_data_sclk,
  tx_data => tx_data_tb,
  transfer_in_progress => transfer_in_progress_tb,
  load_tx_data_ready => load_tx_data_ready_tb,
  rx_data_ready => rx_data_ready_tb,
  spi_miso => spi_data_miso,
  rx_data => rx_data_tb);

debug_spi_data_ncs <= spi_data_ncs;


clock_control : process
begin
  clk <= '0';
  wait for 5 ns;
  clk <= '1';
  wait for 5 ns;
end process clock_control;

reset_control : process
begin
  reset_n <= '0';
  wait for 20 ns;
  reset_n <= '1';
  wait;
end process reset_control;

channel_id_control : process
begin
  channel_id_1 <= "00101";  -- RS-232 (5)
  channel_id_2 <= "00000";  -- N/A (0)
  channel_id_3 <= "00011";  -- CAN (3)
  channel_id_4 <= "00001";  -- GPIO (1)
  channel_id_5 <= "00000";  -- N/A (0)
  channel_id_6 <= "00000";  -- N/A (0)
  wait;
end process channel_id_control;



-- SPI clock 10 MHz -> 100ns period
spi_sclk_control : process
  variable bit_count : natural := 7;
begin
  if (spi_sclk_enabled = '1') then
    spi_data_sclk <= '1';

    wait for 25 ns;

    spi_data_sclk <= '0';

    spi_data_mosi <= spi_byte_to_send(bit_count);
    if (bit_count = 0) then
      bit_count := 7;
    else
      bit_count := bit_count - 1;
    end if;

    wait for 25 ns;
  else
    spi_data_sclk <= '1';
    wait for 25 ns;
    spi_data_mosi <= '0';
    wait for 1 ns;
  end if;
end process spi_sclk_control;

spi_control : process
begin
  spi_sclk_enabled <= '0';
  spi_data_ncs <= '1';
  spi_byte_to_send <= (others => '0');
  wait until reset_n = '1';
  wait for 20 ns;
  -- Transfer start
  spi_data_ncs <= '0';
  -- First byte: status command (0x00)
  spi_byte_to_send <= "00000000";
  wait for 50 ns;
  spi_sclk_enabled <= '1';
  wait for 400 ns;
  spi_sclk_enabled <= '0';
  wait for 3*50 ns;
  spi_sclk_enabled <= '1';

  -- Second byte: dummy (0x00)
  spi_byte_to_send <= "00000000";
  wait for 400 ns;
  spi_sclk_enabled <= '0';
  wait for 3*50 ns;
  spi_sclk_enabled <= '1';

  -- Third byte: dummy (0x00)
  spi_byte_to_send <= "00000000";
  wait for 400 ns;
  spi_sclk_enabled <= '0';
  wait for 3*50 ns;
  spi_sclk_enabled <= '1';

  -- Fourth byte: dummy (0x00)
  spi_byte_to_send <= "00000000";
  wait for 400 ns;

  spi_sclk_enabled <= '0';
  wait for 50 ns;
  spi_data_ncs <= '1';
  wait for 400 ns;

  -- Transfer start
  spi_data_ncs <= '0';
  -- First byte: channel id command (0x12 = 0b0001 0010)
  spi_byte_to_send <= "00010010";
  wait for 50 ns;
  spi_sclk_enabled <= '1';
  wait for 400 ns;
  spi_sclk_enabled <= '0';
  wait for 3*50 ns;
  spi_sclk_enabled <= '1';

  -- Second byte: update all channels
  spi_byte_to_send <= "01111111";
  wait for 400 ns;

  spi_sclk_enabled <= '0';
  wait for 50 ns;
  spi_data_ncs <= '1';
  wait for 400 ns;

  -- Transfer start
  spi_data_ncs <= '0';
  -- First byte: channel id command (0x12 = 0b0001 0010)
  spi_byte_to_send <= "00010010";
  wait for 50 ns;
  spi_sclk_enabled <= '1';
  wait for 400 ns;
  spi_sclk_enabled <= '0';
  wait for 3*50 ns;
  spi_sclk_enabled <= '1';

  -- Second byte: get channel 1
  spi_byte_to_send <= "00000001";
  wait for 400 ns;
  spi_sclk_enabled <= '0';
  wait for 3*50 ns;
  spi_sclk_enabled <= '1';

  -- Third byte: dummy (0x00)
  spi_byte_to_send <= "00000000";
  wait for 400 ns;
  spi_sclk_enabled <= '0';
  wait for 3*50 ns;
  spi_sclk_enabled <= '1';

  -- Fourth byte: dummy (0x00)
  spi_byte_to_send <= "00000000";
  wait for 400 ns;

  spi_sclk_enabled <= '0';
  wait for 50 ns;
  spi_data_ncs <= '1';
  wait for 400 ns;


  wait until reset_n = '0';

end process spi_control;

END bdf_type;