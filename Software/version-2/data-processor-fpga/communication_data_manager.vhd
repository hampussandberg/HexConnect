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
    clk       : in std_logic;
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
    
    -- SPI Slave Interface
    rx_data_ready         : in  std_logic;
    rx_data               : in  std_logic_vector(7 downto 0);
    load_tx_data_ready    : in  std_logic;
    load_tx_data          : out std_logic;
    tx_data               : out std_logic_vector(7 downto 0);
    transfer_in_progress  : in  std_logic;
    
    -- Debug
    debug_leds : out std_logic_vector(7 downto 0));
end communication_data_manager;



architecture behav of communication_data_manager is
  type state_type is (COMMAND, DATA, WAIT_FOR_LOAD_TX_DATA_READY, RETURN_BYTE);
  signal current_state : state_type;

  subtype command_type is std_logic_vector(7 downto 0);
  signal current_command                    : command_type;
  constant STATUS_COMMAND                   : command_type := x"00";
  constant CHANNEL_POWER_COMMAND            : command_type := x"10";
  constant CHANNEL_OUTPUT_COMMAND           : command_type := x"11";
  constant CHANNEL_ID_COMMAND               : command_type := x"12";
  constant CAN_CHANNEL_TERMINATION_COMMAND  : command_type := x"30";
  constant NO_COMMAND                       : command_type := x"FF";

  constant gpio_channel_id : std_logic_vector(4 downto 0)   := "00001";
  constant can_channel_id : std_logic_vector(4 downto 0)    := "00011";
  constant rs_232_channel_id : std_logic_vector(4 downto 0) := "00101";

  signal status              : std_logic_vector(7 downto 0);

  signal channel_direction_a : std_logic_vector(5 downto 0);
  signal channel_direction_b : std_logic_vector(5 downto 0);

  signal channel_termination : std_logic_vector(5 downto 0);
  
  signal channel_id_update_internal     : std_logic_vector(5 downto 0)  := "000000";
  signal channel_power_internal         : std_logic_vector(5 downto 0)  := "000000";
  signal channel_pin_c_output_internal  : std_logic_vector(5 downto 0)  := "000000";
  
  signal load_tx_data_ready_synced    : std_logic := '0';
  signal rx_data_synced               : std_logic_vector(7 downto 0);
  signal rx_data_ready_synced         : std_logic := '0';
  signal rx_data_ready_synced_last    : std_logic := '0';
  signal transfer_in_progress_synced  : std_logic := '0';
  
  signal count          : natural range 0 to 1000000000;
  signal active_channel : natural range 1 to 6;
begin
  process(clk, reset_n)
  begin
    -- Asynchronous reset
    if (reset_n = '0') then
      load_tx_data <= '0';
      tx_data <= (others => '0');
    
      current_command <= NO_COMMAND;

      status <= "00000001";
      
      channel_direction_a <= "000000";
      channel_direction_b <= "000000";
      
      channel_termination <= "000000";
      
      channel_id_update_internal    <= "000000";
      channel_power_internal        <= "000000";
      channel_pin_c_output_internal <= "000000";

      load_tx_data_ready_synced <= '0';
      rx_data_synced <= (others => '0');
      rx_data_ready_synced <= '0';
      rx_data_ready_synced_last <= '0';
      transfer_in_progress_synced <= '0';
      
      count <= 0;
      active_channel <= 1;

      debug_leds <= (others => '0');
      
    -- Synchronous part
    elsif rising_edge(clk) then
      -- Clear the debug leds
      debug_leds <= (others => '0');


      -- TODO: Handle these
      status <= "00000001";
      channel_direction_a <= "000000";
      channel_direction_b <= "000000";
    
      -- Change channel every 10 second
      if (count = 1000000000) then
        count <= 0;
        if (active_channel = 6) then
          active_channel <= 1;
        else
          active_channel <= active_channel + 1;
        end if;
      else
        count <= count + 1;
      end if;
      
      -- Synchronize and store last value
      load_tx_data_ready_synced <= load_tx_data_ready;
      rx_data_synced <= rx_data;
      rx_data_ready_synced <= rx_data_ready;
      rx_data_ready_synced_last <= rx_data_ready_synced;
      transfer_in_progress_synced <= transfer_in_progress;

      -- DEBUG
      --debug_leds <= rx_data_synced;
      debug_leds(5 downto 0) <= channel_termination(5 downto 0);
      
      -- If the transfer is not in progress we should reset the state machine
      if (transfer_in_progress_synced = '0') then
        current_state <= COMMAND;
        current_command <= NO_COMMAND;
      else
        -- COMMAND State ******************************************************
        if (current_state = COMMAND) then
          -- Wait until data is available
          if (rx_data_ready_synced_last = '0' and rx_data_ready_synced = '1') then
            current_command <= rx_data_synced;

            -- If it's the status command we just send back the status
            if (rx_data_synced = STATUS_COMMAND) then
              tx_data       <= status;
              current_state <= WAIT_FOR_LOAD_TX_DATA_READY;
            -- Otherwise read the incoming data byte
            else
              current_state <= DATA;
             end if;
          else
            current_state <= COMMAND;
          end if;

        -- DATA State *********************************************************
        elsif (current_state = DATA) then
          -- Wait until data is available
          if (rx_data_ready_synced_last = '0' and rx_data_ready_synced = '1') then
            -- =========== Channel Power Command ==============================
            if (current_command = CHANNEL_POWER_COMMAND) then
              -- Return Current Channel Power
              if (rx_data_synced(7 downto 6) = "00") then
                tx_data <= "00" & channel_power_internal;
                current_state <= WAIT_FOR_LOAD_TX_DATA_READY;
              -- Enable power
              elsif (rx_data_synced(7 downto 6) = "01") then
                channel_power_internal <= channel_power_internal or 
                                          rx_data_synced(5 downto 0);
                current_state <= COMMAND;
              -- Disable Power
              elsif (rx_data_synced(7 downto 6) = "10") then
                channel_power_internal <= channel_power_internal and 
                                          not rx_data_synced(5 downto 0);
                current_state <= COMMAND;
              end if;
              
            -- =========== Channel Output Command =============================
            elsif (current_command = CHANNEL_OUTPUT_COMMAND) then
              -- Return Current Channel Output
              if (rx_data_synced(7 downto 6) = "00") then
                tx_data <= "00" & channel_pin_c_output_internal;
                current_state <= WAIT_FOR_LOAD_TX_DATA_READY;
              -- Enable output
              elsif (rx_data_synced(7 downto 6) = "01") then
                channel_pin_c_output_internal <=  channel_pin_c_output_internal 
                                                  or rx_data_synced(5 downto 0);
                current_state <= COMMAND;
              -- Disable output
              elsif (rx_data_synced(7 downto 6) = "10") then
                channel_pin_c_output_internal <=  channel_pin_c_output_internal and 
                                                  not rx_data_synced(5 downto 0);
                current_state <= COMMAND;
              end if;

            -- =========== Channel ID Command =================================
            elsif (current_command = CHANNEL_ID_COMMAND) then
              -- Return Current Channel ID
              if (rx_data_synced(7 downto 6) = "00") then
                if (rx_data_synced(0) = '1') then
                  tx_data <= "000" & channel_id_1;
                elsif (rx_data_synced(1) = '1') then
                  tx_data <= "000" & channel_id_2;
                elsif (rx_data_synced(2) = '1') then
                  tx_data <= "000" & channel_id_3;
                elsif (rx_data_synced(3) = '1') then
                  tx_data <= "000" & channel_id_4;
                elsif (rx_data_synced(4) = '1') then
                  tx_data <= "000" & channel_id_5;
                elsif (rx_data_synced(5) = '1') then
                  tx_data <= "000" & channel_id_6;
                else
                  tx_data <= (others => '0');
                end if;
                current_state <= WAIT_FOR_LOAD_TX_DATA_READY;
              -- Update channel ID
              elsif (rx_data_synced(7 downto 6) = "01") then
                channel_id_update_internal <= channel_id_update_internal 
                                              or rx_data_synced(5 downto 0);
                current_state <= COMMAND;
              -- Stop Updating Channel ID
              elsif (rx_data_synced(7 downto 6) = "10") then
                channel_id_update_internal <= channel_id_update_internal and 
                                              not rx_data_synced(5 downto 0);
                current_state <= COMMAND;
              end if;
              
            -- =========== CAN - Channel termination Command ==================
            elsif (current_command = CAN_CHANNEL_TERMINATION_COMMAND) then
              -- Return Current Channel Termination
              if (rx_data_synced(7 downto 6) = "00") then
                tx_data <= "00" & channel_termination;
                current_state <= WAIT_FOR_LOAD_TX_DATA_READY;
              -- Enable Termination
              elsif (rx_data_synced(7 downto 6) = "01") then
                channel_termination <=  channel_termination or 
                                        rx_data_synced(5 downto 0);
                current_state <= COMMAND;
              -- Disable Termination
              elsif (rx_data_synced(7 downto 6) = "10") then
                channel_termination <=  channel_termination and 
                                        not rx_data_synced(5 downto 0);
                current_state <= COMMAND;
              end if;
              
            -- =========== Unknown command ====================================
            else
              current_state <= COMMAND;
            end if;
            -- ================================================================
          else
            current_state <= DATA;
          end if;

        -- WAIT_FOR_LOAD_TX_DATA_READY state **********************************
        elsif (current_state = WAIT_FOR_LOAD_TX_DATA_READY) then
          -- Wait until we can load the tx data
          if (load_tx_data_ready_synced = '1') then
            load_tx_data <= '1';
            current_state <= RETURN_BYTE;
          else
            current_state <= WAIT_FOR_LOAD_TX_DATA_READY;
          end if;

        -- RETURN_BYTE state **************************************************
        elsif (current_state = RETURN_BYTE) then
          -- Wait until data is available, should just be a dummy byte
          if (rx_data_ready_synced_last = '0' and rx_data_ready_synced = '1') then
            tx_data <= (others => '0');
            load_tx_data <= '0';
            current_state <= COMMAND;
          else
            current_state <= RETURN_BYTE;
          end if;

        -- Just in case *******************************************************
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
  
  -- Channel Power
  channel_power         <= channel_power_internal;
  -- Channel Output
  channel_pin_c_output  <= channel_pin_c_output_internal;
  -- Channel ID update
  channel_id_update     <= channel_id_update_internal;
  
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