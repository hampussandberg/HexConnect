-- *******************************************************************************
-- * @file    channel_io_selector.vhd
-- * @author  Hampus Sandberg
-- * @version 0.1
-- * @date    2015-11-16
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
entity channel_io_selector is
  port(
    channel_id : in std_logic_vector(4 downto 0);

    channel_pin_e : inout std_logic;
    channel_pin_f : inout std_logic;

    channel_direction_a : in std_logic;
    channel_direction_b : in std_logic;
    channel_termination : in std_logic);
end channel_io_selector;

architecture behav of channel_io_selector is
  constant gpio_channel_id    : std_logic_vector(4 downto 0) := "00001";
  constant can_channel_id     : std_logic_vector(4 downto 0) := "00011";
  constant rs_232_channel_id  : std_logic_vector(4 downto 0) := "00101";
begin
  
  -- Channel E pin multiplexing
  channel_pin_e <= 
    channel_direction_b when channel_id = gpio_channel_id else 
    'Z';

  -- Channel F pin multiplexing
  channel_pin_f <= 
    channel_direction_a when channel_id = gpio_channel_id else 
    channel_termination when channel_id = can_channel_id else 
    'Z';

end architecture behav;