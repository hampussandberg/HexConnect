-- *******************************************************************************
-- * @file    reset_synchronizer.vhd
-- * @author  Hampus Sandberg
-- * @version 0.1
-- * @date    2015-11-19
-- * @brief   See the paper "Synchronous Resets? Asynchronous Resets? I am so 
-- *          confused! How will I ever know which to use?" by Clifford E. 
-- *          Cummings and Don Mills for more information
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

entity reset_synchronizer is
  port(
    clk               : in  std_logic;
    external_reset_n  : in  std_logic;
    master_reset_n    : out std_logic);
end reset_synchronizer;

architecture behav of reset_synchronizer is
  signal internal_master_reset_n : std_logic;
begin
  process(clk)
  begin
    -- Asynchronous reset assertion
    if (external_reset_n = '0') then
      master_reset_n          <= '0';
      internal_master_reset_n <= '0';

    elsif rising_edge(clk) then
      internal_master_reset_n <= '1';
      master_reset_n          <= internal_master_reset_n;
    end if;
  end process;

end architecture behav;