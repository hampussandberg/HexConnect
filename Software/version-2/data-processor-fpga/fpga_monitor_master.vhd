-- *******************************************************************************
-- * @file    fpga_monitor_master.vhd
-- * @author  Hampus Sandberg
-- * @version 0.1
-- * @date    2015-11-01
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
entity fpga_monitor_master is
  port(
    clk       : in  std_logic;
    reset_n   : in  std_logic;

    -- External interface


    -- Internal interface
    mosi      : out std_logic_vector(7 downto 0);
    miso      : in  std_logic_vector(7 downto 0);
    cs        : out std_logic_vector(7 downto 0)
    );
end fpga_monitor_master;



architecture behav of fpga_monitor_master is
begin
  mosi <= (others => '0');
  cs <= (others => '0');

  process(clk, reset_n)
  begin
    -- Asynchronous reset
    if (reset_n = '0') then
      
    -- Synchronous part
    elsif rising_edge(clk) then
    
    end if; -- if (reset_n = '0')
  end process;

end architecture behav;