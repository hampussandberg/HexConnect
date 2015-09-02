-- *******************************************************************************
-- * @file    pcf8574_controller.vhd
-- * @author  Hampus Sandberg
-- * @version 0.1
-- * @date    2015-09-02
-- * @brief   All channels are configured as outputs
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
entity pcf8574_controller is
  generic(
    device_address  : std_logic_vector(6 downto 0) := "0100000");
  port(
    clk         : in std_logic;
    reset_n     : in std_logic;
    
    -- Inputs for each channel 
    p0  : in std_logic;
    p1  : in std_logic;
    p2  : in std_logic;
    p3  : in std_logic;
    p4  : in std_logic;
    p5  : in std_logic;
    p6  : in std_logic;
    p7  : in std_logic;
    
    -- I2C master Interface
    address   : out  std_logic_vector (7 downto 0);
    data      : out  std_logic_vector (7 downto 0);
    enable    : out  std_logic;
    busy      : in std_logic);
end pcf8574_controller;

architecture behav of pcf8574_controller is
  type state_type is (IDLE, WAIT_FOR_NOT_BUSY, TRANSMITTING);
  signal current_state : state_type := IDLE;
  signal last_p0 : std_logic := '0';
  signal last_p1 : std_logic := '0';
  signal last_p2 : std_logic := '0';
  signal last_p3 : std_logic := '0';
  signal last_p4 : std_logic := '0';
  signal last_p5 : std_logic := '0';
  signal last_p6 : std_logic := '0';
  signal last_p7 : std_logic := '0';
begin
  -- Set the addres to the generic with the LSB set to write (0)
  address <= device_address & '0';

  process(clk, reset_n)
  begin
    -- Asynchronous reset
    if (reset_n = '0') then
      current_state <= IDLE;
      enable <= '0';
      data <= (others => '0');
      last_p0 <= '0';
      last_p1 <= '0';
      last_p2 <= '0';
      last_p3 <= '0';
      last_p4 <= '0';
      last_p5 <= '0';
      last_p6 <= '0';
      last_p7 <= '0';
      
    -- Synchronous part
    elsif rising_edge(clk) then    
      case current_state is
        when IDLE =>
          -- Store the channel value for next time
          last_p0 <= p0;
          last_p1 <= p1;
          last_p2 <= p2;
          last_p3 <= p3;
          last_p4 <= p4;
          last_p5 <= p5;
          last_p6 <= p6;
          last_p7 <= p7;
          -- Check if any of the channels have been updated
          if (last_p0 /= p0 or last_p1 /= p1 or last_p2 /= p2 or last_p3 /= p3 or 
              last_p4 /= p4 or last_p5 /= p5 or last_p6 /= p6 or last_p7 /= p7) then
            data <= p7 & p6 & p5 & p4 & p3 & p2 & p1 & p0;
            current_state <= WAIT_FOR_NOT_BUSY;
          end if;
        
        -- Wait until the I2C master is ready
        when WAIT_FOR_NOT_BUSY =>
          if (busy = '0') then
            enable <= '1';
            current_state <= TRANSMITTING;
          end if;
          
        -- Transmit the data
        when TRANSMITTING =>
          -- When the busy signal goes HIGH we can return to the IDLE state as the data is latched by the I2C master
          if (busy = '1') then
            current_state <= IDLE;
            enable <= '0';
          end if;
      end case;
      
    end if; -- if (reset_n = '0')
  end process;
end architecture behav;