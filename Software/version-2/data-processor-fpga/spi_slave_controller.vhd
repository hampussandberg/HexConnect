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
    -- NOW: CPOL = 1, CPHA = 1
    -- -> Sample MOSI on rising edge and set MISO on falling edge
  port(
    clk           : in std_logic;
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
  signal tx_next                      : std_logic_vector(7 downto 0);
  signal rx_buffer                    : std_logic_vector(7 downto 0);
  signal load_tx_data_ready_internal  : std_logic;
  signal rx_data_ready_internal       : std_logic;
begin
  -- ==========================================================================
  -- CLK Domain
  -- ==========================================================================
  load_rx_process : process(reset_n, clk, spi_cs_n)
  begin
    -- Async clear
    if (reset_n = '0' or spi_cs_n = '1') then
      rx_data <= (others => '0');
    elsif (reset_n = '0') then
      rx_data_ready <= '0';
      
    -- Rising edge of CLK
    elsif (rising_edge(clk)) then
      -- If there is data ready internally we should send it to the rx_data register and notify about it
      if (rx_data_ready_internal = '1') then
        rx_data <= rx_buffer;
        rx_data_ready <= '1';
      else
        rx_data <= (others => '0');
        rx_data_ready <= '0';
      end if;

    end if;

  end process load_rx_process;

  -- ==========================================================================
  -- SPI_SLCK Domain
  -- ==========================================================================
  process(reset_n, spi_sclk, spi_cs_n, load_tx_data, tx_buffer, tx_data, load_tx_data_ready_internal, bit_count, rx_data_ready_internal, rx_buffer)
  begin

    -- Next TX
    if (reset_n = '0' or spi_cs_n = '1') then -- Reset or no transfer
      tx_next <= (others => '0');
    -- Clear it when we are not ready to load
    elsif (load_tx_data_ready_internal = '0') then
      tx_next <= (others => '0');
    -- Load it with the value provided by the user when requested to do so
    elsif (rising_edge(load_tx_data)) then
      tx_next <= tx_data;
    end if;

    -- TX Buffer
    if (reset_n = '0' or spi_cs_n = '1') then -- Reset or no transfer
      tx_buffer <= (others => '0');
    -- Load the buffer at the last rising edge of the spi clock so that it's ready at the next falling edge
    elsif (bit_count = 7 and rising_edge(spi_sclk)) then
      tx_buffer <= tx_next;
    end if;

    -- TX Load Data Ready Internal Register
    if (bit_count = 0) then
      load_tx_data_ready_internal <= '0';
    else
      load_tx_data_ready_internal <= '1';
    end if;

    -- SPI MISO register
    if (reset_n = '0' or spi_cs_n = '1') then -- Reset or no transfer
      spi_miso <= '0';
    -- Set the register to the correct bit in the tx buffer at the falling edge of the spi clock
    elsif (spi_cs_n = '0' and falling_edge(spi_sclk)) then
      spi_miso <= tx_buffer(7 - bit_count);
    end if;

    -- RX buffer register
    if (reset_n = '0') then
      rx_buffer <= (others => '0');
    -- Shift in a new bit when spi transfer is active and there is a rising edge on spi_sclk
    elsif (spi_cs_n = '0' and rising_edge(spi_sclk)) then
      rx_buffer <= rx_buffer(6 downto 0) & spi_mosi;
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
    if (reset_n = '0' or spi_cs_n = '1') then
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

end architecture behav;