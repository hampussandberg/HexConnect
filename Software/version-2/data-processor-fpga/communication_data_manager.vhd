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
    
    -- Channel ID
    channel_id_1 : in std_logic_vector(4 downto 0);
    channel_id_2 : in std_logic_vector(4 downto 0);
    channel_id_3 : in std_logic_vector(4 downto 0);
    channel_id_4 : in std_logic_vector(4 downto 0);
    channel_id_5 : in std_logic_vector(4 downto 0);
    channel_id_6 : in std_logic_vector(4 downto 0);
    channel_id_update : out std_logic_vector(5 downto 0);
    
    -- Channel Power Control
    channel_power : out std_logic_vector(5 downto 0);
    
    -- Channel Output Switching, pin C
    channel_pin_c_output : out std_logic_vector(5 downto 0);
    
    -- Channel E & F pins
    channel_pin_e : out std_logic_vector(5 downto 0);
    channel_pin_f : out std_logic_vector(5 downto 0);
    
    -- Debug
    debug_leds : out std_logic_vector(7 downto 0));
end communication_data_manager;



architecture behav of communication_data_manager is
  constant gpio_channel_id : std_logic_vector(4 downto 0)   := "00001";
  constant can_channel_id : std_logic_vector(4 downto 0)    := "00011";
  constant rs_232_channel_id : std_logic_vector(4 downto 0) := "00101";

  signal channel_direction_a : std_logic_vector(5 downto 0);
  signal channel_direction_b : std_logic_vector(5 downto 0);

  signal channel_termination : std_logic_vector(5 downto 0);
begin
	process(clk, reset_n)
    variable count : integer range 0 to 1000000000;
    variable active_channel : integer range 1 to 6;
	begin
		-- Asynchronous reset
		if (reset_n = '0') then
      channel_id_update <= "111111";
      channel_power <= "000000";
      channel_pin_c_output <= "000000";
      count := 0;
      active_channel := 1;
      
		-- Synchronous part
		elsif rising_edge(clk) then
      -- Change channel every 10 second
      if (count = 1000000000) then
				count := 0;
				if (active_channel = 6) then
          active_channel := 1;
        else
          active_channel := active_channel + 1;
        end if;
			else
				count := count + 1;
			end if;
      
      
      if (active_channel = 1) then
        channel_power <= "000001";
        debug_leds(5 downto 1) <= channel_id_1;
      elsif (active_channel = 2) then
        channel_power <= "000010";
        debug_leds(5 downto 1) <= channel_id_2;
      elsif (active_channel = 3) then
        channel_power <= "000100";
        debug_leds(5 downto 1) <= channel_id_3;
      elsif (active_channel = 4) then
        channel_power <= "001000";
        debug_leds(5 downto 1) <= channel_id_4;
      elsif (active_channel = 5) then
        channel_power <= "010000";
        debug_leds(5 downto 1) <= channel_id_5;
      elsif (active_channel = 6) then
        channel_power <= "100000";
        debug_leds(5 downto 1) <= channel_id_6;
      else
        active_channel := 1;
        channel_power <= "000001";
        debug_leds(5 downto 1) <= channel_id_1;
      end if;
    
		end if; -- if (reset_n = '0')
	end process;
  
  -- Channel E pin multiplexing
  channel_pin_e(0) <= 
    channel_direction_b(0) when channel_id_1 = gpio_channel_id else 
    'Z';
  channel_pin_e(1) <= 
    channel_direction_b(1) when channel_id_2 = gpio_channel_id else 
    'Z';
  channel_pin_e(2) <= 
    channel_direction_b(2) when channel_id_3 = gpio_channel_id else 
    'Z';
  channel_pin_e(3) <= 
    channel_direction_b(3) when channel_id_4 = gpio_channel_id else 
    'Z';
  channel_pin_e(4) <= 
    channel_direction_b(4) when channel_id_5 = gpio_channel_id else 
    'Z';
  channel_pin_e(5) <= 
    channel_direction_b(5) when channel_id_6 = gpio_channel_id else 
    'Z';

  -- Channel F pin multiplexing
  channel_pin_f(0) <= 
    channel_direction_a(0) when channel_id_1 = gpio_channel_id else 
    channel_termination(0) when channel_id_1 = can_channel_id else 
    'Z';
  channel_pin_f(1) <= 
    channel_direction_a(1) when channel_id_2 = gpio_channel_id else 
    channel_termination(1) when channel_id_2 = can_channel_id else 
    'Z';
  channel_pin_f(2) <= 
    channel_direction_a(2) when channel_id_3 = gpio_channel_id else 
    channel_termination(2) when channel_id_3 = can_channel_id else 
    'Z';
  channel_pin_f(3) <= 
    channel_direction_a(3) when channel_id_4 = gpio_channel_id else 
    channel_termination(3) when channel_id_4 = can_channel_id else 
    'Z';
  channel_pin_f(4) <= 
    channel_direction_a(4) when channel_id_5 = gpio_channel_id else 
    channel_termination(4) when channel_id_5 = can_channel_id else 
    'Z';
  channel_pin_f(5) <= 
    channel_direction_a(5) when channel_id_6 = gpio_channel_id else 
    channel_termination(5) when channel_id_6 = can_channel_id else 
    'Z';

end architecture behav;