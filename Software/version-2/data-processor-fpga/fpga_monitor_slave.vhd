-- *******************************************************************************
-- * @file    fpga_monitor_slave.vhd
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
entity fpga_monitor_slave is
  port(
    clk       : in  std_logic;
    reset_n   : in  std_logic;

    -- Communication interface
    mosi      : in  std_logic_vector(7 downto 0);
    miso_out  : out std_logic_vector(7 downto 0);
    miso_in   : in  std_logic_vector(7 downto 0);
    cs        : in  std_logic;

    -- Register read interface
    reg_0     : in  std_logic_vector(7 downto 0);
    reg_1     : in  std_logic_vector(7 downto 0);
    reg_2     : in  std_logic_vector(7 downto 0);
    reg_3     : in  std_logic_vector(7 downto 0);
    reg_4     : in  std_logic_vector(7 downto 0);
    reg_5     : in  std_logic_vector(7 downto 0);
    reg_6     : in  std_logic_vector(7 downto 0);
    reg_7     : in  std_logic_vector(7 downto 0);
    reg_8     : in  std_logic_vector(7 downto 0);
    reg_9     : in  std_logic_vector(7 downto 0);
    reg_a     : in  std_logic_vector(7 downto 0);
    reg_b     : in  std_logic_vector(7 downto 0);
    reg_c     : in  std_logic_vector(7 downto 0);
    reg_d     : in  std_logic_vector(7 downto 0);
    reg_e     : in  std_logic_vector(7 downto 0);
    reg_f     : in  std_logic_vector(7 downto 0)
    );
end fpga_monitor_slave;



architecture behav of fpga_monitor_slave is
  signal miso_internal : std_logic_vector(7 downto 0);
begin
  process(clk, reset_n)
  begin
    -- Asynchronous reset
    if (reset_n = '0') then
      miso_internal <= (others => '0');
    -- Synchronous part
    elsif rising_edge(clk) then
    
    end if; -- if (reset_n = '0')
  end process;

  -- Output on MISO when slave is selected
  miso_out <= miso_internal when cs = '1' else
              miso_in;

end architecture behav;