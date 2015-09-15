-- *******************************************************************************
-- * @file    id_formatter.vhd
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
entity id_formatter is
  port(
    clk         : in std_logic;
    reset_n       : in std_logic;
    
    update_id   : in std_logic_vector(5 downto 0);
    
    -- ADC Interface  
    ch0_value    : in std_logic_vector(9 downto 0);
    ch1_value    : in std_logic_vector(9 downto 0);
    ch2_value    : in std_logic_vector(9 downto 0);
    ch3_value    : in std_logic_vector(9 downto 0);
    ch4_value    : in std_logic_vector(9 downto 0);
    ch5_value    : in std_logic_vector(9 downto 0);
    ch6_value    : in std_logic_vector(9 downto 0);
    ch7_value    : in std_logic_vector(9 downto 0);
    
    -- Channel ID interface  
    channel_id_1  : out std_logic_vector(4 downto 0);
    channel_id_2  : out std_logic_vector(4 downto 0);
    channel_id_3  : out std_logic_vector(4 downto 0);
    channel_id_4  : out std_logic_vector(4 downto 0);
    channel_id_5  : out std_logic_vector(4 downto 0);
    channel_id_6  : out std_logic_vector(4 downto 0);
    valid_ids    : out std_logic);
end id_formatter;

architecture behav of id_formatter is
  signal ids_have_been_saved : std_logic;
begin
  process(clk, reset_n)
  begin
    -- Asynchronous reset
    if (reset_n = '0') then
      ids_have_been_saved <= '0';
      
      channel_id_1 <= (others => '0');
      channel_id_2 <= (others => '0');
      channel_id_3 <= (others => '0');
      channel_id_4 <= (others => '0');
      channel_id_5 <= (others => '0');
      channel_id_6 <= (others => '0');
      valid_ids <= '0';
    
    -- Synchronous part
    elsif rising_edge(clk) then      
      -- If the Ids have not been saved before we should save the values
      if (ids_have_been_saved = '0') then
        ids_have_been_saved <= '1';
        channel_id_1 <= ch0_value(9 downto 5);
        channel_id_2 <= ch1_value(9 downto 5);
        channel_id_3 <= ch2_value(9 downto 5);
        channel_id_4 <= ch3_value(9 downto 5);
        channel_id_5 <= ch4_value(9 downto 5);
        channel_id_6 <= ch5_value(9 downto 5);
        valid_ids <= '1';
      end if;
      
      -- Update a channel value if instructed to do so
      if (update_id(0) = '1') then
        channel_id_1 <= ch0_value(9 downto 5);
      end if;
      if (update_id(1) = '1') then
        channel_id_2 <= ch1_value(9 downto 5);
      end if;
      if (update_id(2) = '1') then
        channel_id_3 <= ch2_value(9 downto 5);
      end if;
      if (update_id(3) = '1') then
        channel_id_4 <= ch3_value(9 downto 5);
      end if;
      if (update_id(4) = '1') then
        channel_id_5 <= ch4_value(9 downto 5);
      end if;
      if (update_id(5) = '1') then
        channel_id_6 <= ch5_value(9 downto 5);
      end if;
      
    end if; -- if (reset_n = '0')
  end process;
end architecture behav;