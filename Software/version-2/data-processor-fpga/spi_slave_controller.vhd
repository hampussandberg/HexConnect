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
  generic(
    DATA_WIDTH     : integer := 8;    -- Amount of data for each transfer
    CLK_PRESCALER  : integer := 32);    -- Divide clk with this to get spi_sclk
    -- TODO: Clock polarity, etc...
    -- NOW: CPOL = 1, CPHA = 1
  port(
    clk           : in std_logic;
    reset_n       : in std_logic;
    
    -- Module interface
    data_to_send        : in std_logic_vector(DATA_WIDTH-1 downto 0);
    data_to_send_valid  : in std_logic;
    data_received_valid : out std_logic;
    data_received       : out std_logic_vector(DATA_WIDTH-1 downto 0);
    busy_transfer       : out std_logic;
    
    -- External hardware interface
    spi_data_in     : in std_logic;
    spi_cs_n        : in std_logic;
    spi_sclk        : in std_logic;
    spi_data_out    : out std_logic);
end spi_slave_controller;

architecture behav of spi_slave_controller is
begin
  process(clk, reset_n)
  begin
    -- Asynchronous reset
    if (reset_n = '0') then
    
    -- Synchronous part
    elsif rising_edge(clk) then
  
    end if; -- if (reset_n = '0')
  end process;

end architecture behav;