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
    clk           : in std_logic;
    reset_n       : in std_logic;
    
    -- Module interface
    data_to_send        : in std_logic_vector(7 downto 0);
    data_to_send_valid  : in std_logic;
    data_received_valid : out std_logic;
    data_received       : out std_logic_vector(7 downto 0);
    busy_transfer       : out std_logic;
    
    -- External hardware interface
    spi_mosi  : in std_logic;
    spi_cs_n  : in std_logic;
    spi_sclk  : in std_logic;
    spi_miso  : out std_logic);
end spi_slave_controller;

architecture behav of spi_slave_controller is
  signal spi_mosi_synced      : std_logic;
  signal spi_cs_n_synced      : std_logic;
  signal spi_cs_n_synced_last : std_logic;
  signal spi_sclk_synced      : std_logic;
  signal spi_sclk_synced_last : std_logic;
  
  signal bit_count : natural range 0 to 7;
  signal receivedByte : std_logic_vector(7 downto 0);
  signal transmittedByte : std_logic_vector(7 downto 0);
begin
  process(clk, reset_n)
  begin
    -- Asynchronous reset
    if (reset_n = '0') then
      data_received_valid <= '0';
    
      spi_mosi_synced <= '0';
      spi_cs_n_synced <= '0';
      spi_cs_n_synced_last <= '0';
      spi_sclk_synced <= '0';
      spi_sclk_synced_last <= '0';
      
      receivedByte <= (others => '0');
      transmittedByte <= (others => '0');
      
      bit_count <= 0;
    
    -- Synchronous part
    elsif rising_edge(clk) then
      -- Synchronize the signals
      spi_mosi_synced <= spi_mosi;
      spi_cs_n_synced <= spi_cs_n;
      spi_sclk_synced <= spi_sclk;
      
      -- Store the last value
      spi_cs_n_synced_last <= spi_cs_n_synced;
      spi_sclk_synced_last <= spi_sclk_synced;
      
      -- If the CS pin has a falling edge we should start over
      if (spi_cs_n_synced_last = '1' and spi_cs_n_synced = '0') then
        bit_count <= 0;
        data_received_valid <= '0';
        receivedByte <= (others => '0');
      end if;
      
      -- Store data to send
      if (data_to_send_valid = '1') then
        transmittedByte <= data_to_send;
      end if;
      
      
      -- If the CS pin is low we are processing data
      if (spi_cs_n_synced = '0') then
        -- Sample the MOSI pin on rising edge of SCLK
        if (spi_sclk_synced_last = '0' and spi_sclk_synced = '1') then
          -- Shift in the sampled bit
          receivedByte <= receivedByte(6 downto 0) & spi_mosi_synced;
          -- If we have shifted in 7 bits the receivedByte is valid and we can start over again
          if (bit_count = 7) then
            bit_count <= 0;
            data_received_valid <= '1';
          -- Otherwise we keep shifting the next clock cycle
          else
            bit_count <= bit_count + 1;
            data_received_valid <= '0';
          end if;
          
        -- Output the next bit on falling edge of SCLK
        elsif (spi_sclk_synced_last = '1' and spi_sclk_synced = '0') then
          transmittedByte <= transmittedByte(6 downto 0) & '0';
        end if;
      
      end if;
    end if; -- if (reset_n = '0')
  end process;
  
  -- Transmit MSB first
  spi_miso <= transmittedByte(7);
  
  --
  data_received <= receivedByte;

end architecture behav;