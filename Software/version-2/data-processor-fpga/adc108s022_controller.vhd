-- *******************************************************************************
-- * @file    adc108s022_controller.vhd
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
entity adc108s022_controller is
	port(
		clk 				: in std_logic;
		reset_n 			: in std_logic;
		enable			: in std_logic;
			
		ch0_value		: out std_logic_vector(9 downto 0);
		ch1_value		: out std_logic_vector(9 downto 0);
		ch2_value		: out std_logic_vector(9 downto 0);
		ch3_value		: out std_logic_vector(9 downto 0);
		ch4_value		: out std_logic_vector(9 downto 0);
		ch5_value		: out std_logic_vector(9 downto 0);
		ch6_value		: out std_logic_vector(9 downto 0);
		ch7_value		: out std_logic_vector(9 downto 0);
		valid_values	: out std_logic;
		
		-- SPI Interface
		valid_data 		: in std_logic;
		data_received	: in std_logic_vector(15 downto 0);
		busy_transfer	: in std_logic;
		data_to_send	: out std_logic_vector(15 downto 0);
		start_transfer : out std_logic);
end adc108s022_controller;

architecture behav of adc108s022_controller is
	signal channel_to_sample_next : integer range 0 to 7 := 0;
	signal init_sent : std_logic;
	signal new_transfer_started : std_logic;
	signal last_valid_data : std_logic;
	
	type ADC_STATE is (RESET_STATE, INIT_STATE, RUNNING_STATE);
	signal current_state : ADC_STATE;
begin
	process(clk, reset_n)
	begin
		-- Asynchronous reset
		if (reset_n = '0') then
			data_to_send <= (others => '0');
			start_transfer <= '0';
			
			ch0_value <= (others => '0');
			ch1_value <= (others => '0');
			ch2_value <= (others => '0');
			ch3_value <= (others => '0');
			ch4_value <= (others => '0');
			ch5_value <= (others => '0');
			ch6_value <= (others => '0');
			ch7_value <= (others => '0');
			valid_values <= '0';
		
			channel_to_sample_next <= 0;
			init_sent <= '0';
			new_transfer_started <= '0';
			last_valid_data <= '0';
			
			current_state <= RESET_STATE;
		
		-- Synchronous part
		elsif rising_edge(clk) then
			last_valid_data <= valid_data;
		
			case current_state is
				-- ====================================================================================
				-- When coming out of reset
				-- ====================================================================================
				when RESET_STATE =>
					current_state <= INIT_STATE;
					
				-- ====================================================================================
				-- The channel sent to the ADC is for the next cycle so we need to do a transfer with ch0 first
				-- ====================================================================================
				when INIT_STATE =>
					if (enable = '1' and busy_transfer = '0' and init_sent <= '0') then
						data_to_send(13 downto 11) <= std_logic_vector(to_unsigned(channel_to_sample_next, 3));
						start_transfer <= '1';
						channel_to_sample_next <= channel_to_sample_next + 1;
						init_sent <= '1';
					elsif (busy_transfer = '0' and init_sent <= '1') then
						current_state <= RUNNING_STATE;
					else
						start_transfer <= '0';
					end if; -- (enable = '1' and busy_transfer = '0' and init_sent <= '0')
					
				-- ====================================================================================
				-- Normal running state
				-- ====================================================================================
				when RUNNING_STATE =>
					if (enable = '1' and busy_transfer = '0' and new_transfer_started <= '0') then
						-- If there is a rising edge on the valid_data we should save the data_received to a channel
						if (last_valid_data = '0' and valid_data = '1') then
							case channel_to_sample_next is 
								-- Add the new value to the storage
								-- Data received: | 0 0 0 0 D9 D8 D7 D6 D5 D4 D3 D2 D1 D0 0 0 | 0 0 0 0 D9 D8 ...
								when 0 => ch6_value <= data_received(11 downto 2);
								when 1 => ch7_value <= data_received(11 downto 2);
								when 2 => ch0_value <= data_received(11 downto 2);
								when 3 => ch1_value <= data_received(11 downto 2);
								when 4 => ch2_value <= data_received(11 downto 2);
								when 5 => ch3_value <= data_received(11 downto 2);
								when 6 => ch4_value <= data_received(11 downto 2);
								when 7 => ch5_value <= data_received(11 downto 2);
								when others => null;
							end case;
						end if; -- (last_valid_data = '0' and valid_data = '1')
						
						-- Send the command to sample the next channel
						data_to_send(13 downto 11) <= std_logic_vector(to_unsigned(channel_to_sample_next, 3));
						start_transfer <= '1';
						new_transfer_started <= '1';
						
						-- Start from the beginning again when the last channel has been reached
						if (channel_to_sample_next = 7) then
							channel_to_sample_next <= 0;
							valid_values <= '1';
						else
							channel_to_sample_next <= channel_to_sample_next + 1;
							valid_values <= '0';
						end if; -- (channel_to_sample_next = 7)
					else
						new_transfer_started <= '0';
						start_transfer <= '0';
					end if; -- if (enable = '1' and busy_transfer = '0' and new_transfer_started <= '0')
				-- ====================================================================================
				when others => null;
			end case; -- current_state
		end if; -- if (reset_n = '0')
	end process;

end architecture behav;