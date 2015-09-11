-- *******************************************************************************
-- * @file    communication_data_manager.vhd
-- * @author  Hampus Sandberg
-- * @version 0.1
-- * @date    2015-08-16
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
entity communication_data_manager is
	port(
		clk 			: in std_logic;
		reset_n   : in std_logic;
    
    channel_id_1 : in std_logic_vector(4 downto 0);
    channel_id_2 : in std_logic_vector(4 downto 0);
    channel_id_3 : in std_logic_vector(4 downto 0);
    channel_id_4 : in std_logic_vector(4 downto 0);
    channel_id_5 : in std_logic_vector(4 downto 0);
    channel_id_6 : in std_logic_vector(4 downto 0);
    
    channel_power : out std_logic_vector(5 downto 0);
    channel_output : out std_logic_vector(5 downto 0));
end communication_data_manager;

architecture behav of communication_data_manager is
begin
	process(clk, reset_n)
	begin
		-- Asynchronous reset
		if (reset_n = '0') then
      channel_power <= "010001";
      channel_output <= "000000";
      
		-- Synchronous part
		elsif rising_edge(clk) then
		end if; -- if (reset_n = '0')
	end process;

end architecture behav;