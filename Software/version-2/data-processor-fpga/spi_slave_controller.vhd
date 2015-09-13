-- *******************************************************************************
-- * @file    spi_slave_controller.vhd
-- * @author  Hampus Sandberg
-- * @version 0.1
-- * @date    2015-09-11
-- * @brief
-- *******************************************************************************
--  Copyright (c) 2015 Hampus Sandberg.
--
--  This program is free software: you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation, either version 3 of the License, or
--  any later version.
--
--  This program is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with this program.  If not, see <http://www.gnu.org/licenses/>.
-- *******************************************************************************

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- Entity
entity spi_slave_controller is
    -- TODO: Clock polarity, etc...
    -- NOW: CPOL = 0, CPHA = 0
    -- -> Sample MOSI on rising edge and set MISO on falling edge
  port(
    reset_n       : in std_logic;
    
    -- Module interface
    transfer_in_progress  : out std_logic;
    load_tx_data_ready    : out std_logic;  -- High indicates new data can be loaded into the register
    load_tx_data          : in  std_logic;  -- Rising edge latches tx_data into the register if load_tx_data_ready is high
    tx_data               : in  std_logic_vector(7 downto 0);
    rx_data_ready         : out std_logic;  -- Indicates when internal data is ready
    rx_data               : out std_logic_vector(7 downto 0);
    
    -- External hardware interface
    spi_mosi  : in  std_logic;
    spi_cs_n  : in  std_logic;
    spi_sclk  : in  std_logic;
    spi_miso  : out std_logic);
end spi_slave_controller;

architecture behav of spi_slave_controller is
  signal bit_count                    : natural range 0 to 7 := 0;
  signal tx_buffer                    : std_logic_vector(7 downto 0);
  signal rx_buffer                    : std_logic_vector(7 downto 0);
  signal load_tx_data_ready_internal  : std_logic;
  signal rx_data_ready_internal       : std_logic;
begin
  process(reset_n, spi_sclk, spi_cs_n, load_tx_data, tx_buffer, tx_data, load_tx_data_ready_internal, bit_count, rx_data_ready_internal, rx_buffer)
  begin

    -- SPI MISO register
    if (reset_n = '0' or spi_cs_n = '1') then -- Reset or no transfer
      spi_miso <= '0';
    else
      spi_miso <= tx_buffer(7);
    end if;


    -- TX Buffer register
    if (reset_n = '0') then
      tx_buffer <= (others => '0');
    -- Load the tx buffer when the user requests it and we are ready to do so
    elsif (load_tx_data = '1' and load_tx_data_ready_internal = '1') then
      tx_buffer <= tx_data;
    -- Shift the tx buffer on rising edge of spi_sclk
    elsif (spi_cs_n = '0' and falling_edge(spi_sclk)) then
      tx_buffer <= tx_buffer(6 downto 0) & '0';
    end if;

    -- TX Load Data Ready Internal Register
    if (reset_n = '0') then
      load_tx_data_ready_internal <= '1';
    -- When the user loads the tx buffer we are not ready to load again afterwards
    elsif (load_tx_data = '1' and load_tx_data_ready_internal = '1') then
      load_tx_data_ready_internal <= '0';
    -- When we have reached the final falling edge of spi_sclk we are ready again
    elsif (falling_edge(spi_sclk) and bit_count = 0) then
      load_tx_data_ready_internal <= '1';
    end if;



    -- RX buffer register
    if (reset_n = '0') then
      rx_buffer <= (others => '0');
    -- Shift in a new bit when spi transfer is active and there is a rising edge on spi_sclk
    elsif (spi_cs_n = '0' and rising_edge(spi_sclk)) then
      rx_buffer <= rx_buffer(6 downto 0) & spi_mosi;
    end if;

    -- RX Data output register
    if (reset_n = '0') then
      rx_data <= (others => '0');
    -- Move the buffer to rx data when data is ready
    elsif (rx_data_ready_internal = '1') then
      rx_data <= rx_buffer;
    -- Otherwise set it to 0
    else
      rx_data <= (others => '0');
    end if;

    -- RX Data Ready Internal Register
    if (reset_n = '0') then
      rx_data_ready_internal <= '0';
    -- RX Data is not ready any longer at the first rising edge of spi_sclk
    elsif (bit_count = 0 and rising_edge(spi_sclk)) then
      rx_data_ready_internal <= '0';
    -- RX Data is ready when spi transfer is active and we have reached bit 7 and there is a rising edge on the spi_sclk
    elsif (spi_cs_n = '0' and bit_count = 7 and rising_edge(spi_sclk)) then
      rx_data_ready_internal <= '1';
    end if;


    -- Bit count register
    if (reset_n = '0') then
      bit_count <= 0;
    elsif (rising_edge(spi_sclk)) then
      if (bit_count = 7) then
        bit_count <= 0;
      else
        bit_count <= bit_count + 1;
      end if;
    end if;

  end process;

  transfer_in_progress <= not spi_cs_n;

  load_tx_data_ready <= load_tx_data_ready_internal;
  rx_data_ready <= rx_data_ready_internal;

end architecture behav;