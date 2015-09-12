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
    
    -- SPI Interface
    received_byte         : in std_logic_vector(7 downto 0);
    received_byte_valid   : in std_logic;
    transfer_in_progress  : in std_logic;
    
    -- Debug
    debug_leds : out std_logic_vector(7 downto 0));
end communication_data_manager;



architecture behav of communication_data_manager is
  signal received_byte_valid_last : std_logic;
  type state_type is (COMMAND, DATA);
  signal current_state : state_type;

  
  subtype command_type is std_logic_vector(7 downto 0);
  signal current_command          : command_type;
  constant NO_COMMAND             : command_type := x"00";
  constant CHANNEL_POWER_COMMAND  : command_type := x"10";
  constant CHANNEL_OUTPUT_COMMAND : command_type := x"11";

  constant gpio_channel_id : std_logic_vector(4 downto 0)   := "00001";
  constant can_channel_id : std_logic_vector(4 downto 0)    := "00011";
  constant rs_232_channel_id : std_logic_vector(4 downto 0) := "00101";

  signal channel_direction_a : std_logic_vector(5 downto 0);
  signal channel_direction_b : std_logic_vector(5 downto 0);

  signal channel_termination : std_logic_vector(5 downto 0);
  
  signal channel_power_internal : std_logic_vector(5 downto 0)        := "000000";
  signal channel_pin_c_output_internal : std_logic_vector(5 downto 0) := "000000";
begin
	process(clk, reset_n)
    variable count : integer range 0 to 1000000000;
    variable active_channel : integer range 1 to 6;
	begin
		-- Asynchronous reset
		if (reset_n = '0') then
      received_byte_valid_last <= '0';
      current_command <= NO_COMMAND;
      
      channel_id_update <= "111111";
      channel_power_internal <= "000000";
      channel_pin_c_output_internal <= "000000";
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
      
      -- Store for next cycle
      received_byte_valid_last <= received_byte_valid;
      
      -- If the transfer is not in progress we should reset the state machine
      if (transfer_in_progress = '0') then
        current_state <= COMMAND;
        current_command <= NO_COMMAND;      
      -- Check for rising edge to detect when new byte is available
      elsif (received_byte_valid_last = '0' and received_byte_valid = '1') then
        -- Command State
        if (current_state = COMMAND) then
          current_command <= received_byte;
          current_state <= DATA;
        -- Data state
        elsif (current_state = DATA) then
        
          -- =========== Channel Power Command ===========
          if (current_command = CHANNEL_POWER_COMMAND) then
            -- Enable power
            if (received_byte(7 downto 6) = "01") then
              channel_power_internal <= channel_power_internal or received_byte(5 downto 0);
            -- Disable Power
            elsif (received_byte(7 downto 6) = "10") then
              channel_power_internal <= channel_power_internal and not received_byte(5 downto 0);
            end if;
            current_state <= COMMAND;
            
          -- =========== Channel Output Command ===========
          elsif (current_command = CHANNEL_OUTPUT_COMMAND) then
            -- Enable power
            if (received_byte(7 downto 6) = "01") then
              channel_pin_c_output_internal <= channel_pin_c_output_internal or received_byte(5 downto 0);
            -- Disable Power
            elsif (received_byte(7 downto 6) = "10") then
              channel_pin_c_output_internal <= channel_pin_c_output_internal and not received_byte(5 downto 0);
            end if;
            current_state <= COMMAND;
          -- =========== Unknown command ===========
          else
            current_state <= COMMAND;
          end if;
          
        -- Just in case
        else
          current_state <= COMMAND;
        end if;
      end if;
      
      
      -- ======== TEST ========
--      if (active_channel = 1) then
--        channel_power_internal <= "000001";
--        --debug_leds(5 downto 1) <= channel_id_1;
--      elsif (active_channel = 2) then
--        channel_power_internal <= "000010";
--        --debug_leds(5 downto 1) <= channel_id_2;
--      elsif (active_channel = 3) then
--        channel_power_internal <= "000100";
--        --debug_leds(5 downto 1) <= channel_id_3;
--      elsif (active_channel = 4) then
--        channel_power_internal <= "001000";
--        --debug_leds(5 downto 1) <= channel_id_4;
--      elsif (active_channel = 5) then
--        channel_power_internal <= "010000";
--        --debug_leds(5 downto 1) <= channel_id_5;
--      elsif (active_channel = 6) then
--        channel_power_internal <= "100000";
--        --debug_leds(5 downto 1) <= channel_id_6;
--      else
--        active_channel := 1;
--        channel_power_internal <= "000001";
--        --debug_leds(5 downto 1) <= channel_id_1;
--      end if;
    
		end if; -- if (reset_n = '0')
	end process;
  
  -- ======== TEST ========
  debug_leds <= received_byte;
  
  -- Channel Power
  channel_power <= channel_power_internal;
  -- Channel Output
  channel_pin_c_output <= channel_pin_c_output_internal;
  
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