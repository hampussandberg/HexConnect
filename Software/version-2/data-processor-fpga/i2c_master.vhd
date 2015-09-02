-- *******************************************************************************
-- * @file    i2c_master.vhd
-- * @author  Hampus Sandberg
-- * @version 0.1
-- * @date    2015-09-02
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
use ieee.std_logic_unsigned.all;

entity i2c_master IS
  generic(
    clk_freq          : integer := 100000000;
    i2c_bus_clk_freq  : integer := 400000);
  port(
    clk       : in  std_logic;
    reset_n   : in  std_logic;
    address   : in  std_logic_vector (7 downto 0);
    data      : in  std_logic_vector (7 downto 0);
    i2c_sclk  : out std_logic;
    i2c_sdat  : out std_logic;
    enable    : in  std_logic;
    busy      : out std_logic;
    slave_ack : out std_logic);
end i2c_master ;

--
architecture behav of i2c_master is
  constant divider        : integer := (clk_freq/i2c_bus_clk_freq) / 4; -- Use four internal cycles for each sclk cycle
  type state_type is (ready, start, address_write, slave_ack1, data_write, slave_ack2, stop);
  signal state            : state_type;
  signal state_clk        : std_logic;
  signal internal_sclk    : std_logic;
  signal enable_sclk      : std_logic;
  signal internal_sdat    : std_logic;
  signal bit_count        : integer range 0 to 7;
  signal internal_address : std_logic_vector (7 downto 0);
  signal internal_data    : std_logic_vector (7 downto 0);
  signal sda_enable_n     : std_logic;
begin
  -- ========================================================================
  
  -- Process to generate the i2c_sclk and clock for state machine
  clock_generator : process (clk, reset_n)
    variable count : integer range 0 to divider * 4;
  begin
    -- Reset
    if (reset_n = '0') then
      count := 0;
      state_clk <= '0';
      internal_sclk <= '0';
      
    -- Clocked
    elsif (rising_edge(clk)) then
      -- Check if the counter has reached the end
      if (count = divider*4 - 1) then
        count := 0;
      -- Otherwise increment the counter
      else
        count := count + 1;
      end if;
      
      -- First 1/4, clock is LOW and state machine should not do anything
      if (count >= 0 and count <= divider - 1) then
        internal_sclk <= '0';
        state_clk <= '0';
      -- Second 1/4, clock is still LOW but state machine should run
      elsif (count >= divider and count <= divider*2 - 1) then
        internal_sclk <= '0';
        state_clk <= '1';
      -- Third 1/4, clock will change to HIGH as we now are in the middle.
      -- State machine should not run
      elsif (count >= divider*2 and count <= divider*3 - 1) then
        internal_sclk <= '1';
        state_clk <= '1';
      -- Fourth 1/4, clock should stay HIGH, state machine should not run, but reset it's clock
      elsif (count >= divider*3 and count <= divider*4 - 1) then
        internal_sclk <= '1';
        state_clk <= '0';
      end if;
      
    end if;
  end process clock_generator;
  
  -- ========================================================================
  
  state_machine : process (state_clk, reset_n)
  begin
    -- Reset
    if (reset_n = '0') then
      busy          <= '0';	-- start as not busy
      state         <= ready;	-- start in ready state
      enable_sclk   <= '0';	-- disable the clock
      internal_sdat <= '1';	-- set sda high impedance
      bit_count     <= 7;
      internal_address <= (others => '0');
      internal_data <= (others => '0');
      slave_ack <= '0';

    -- On rising edge of internal state clock
    elsif (rising_edge(state_clk)) then
      case (state) is        
        -- Ready state
        when ready =>
          if (enable = '1') then
            busy <= '1';
            internal_address <= address;
            internal_data <= data;
            internal_sdat <= '0'; -- Pull low to indicate start
            bit_count <= 7;
            state <= start; -- Go to next state
          else
            busy <= '0';
            state <= ready;
          end if;
        
        -- Start state
        when start =>
          busy <= '1';
          enable_sclk <= '1';	-- Enable the output of the clock
          internal_sdat <= internal_address(bit_count); -- Start writing the address
          state <= address_write; -- Go to next state
        
        -- Address state
        when address_write =>
          if (bit_count = 0) then
            internal_sdat <= '1'; -- Release the bus so that the slave can acknowledge the address
            bit_count <= 7;
            state <= slave_ack1; -- Go to next state
            slave_ack <= '1';
          else
            bit_count <= bit_count - 1; -- Decrease the bit counter
            internal_sdat <= internal_address(bit_count - 1); -- Continue writing the address
            state <= address_write; -- Stay in the same state
          end if;

        -- Slave ack 1 state, we don't care about the ack, instead we just keep going
        when slave_ack1 =>
          internal_sdat <= internal_data(bit_count);
          state <= data_write; -- Go the next state
          slave_ack <= '0';

        -- Data write state
        when data_write =>
          if (bit_count = 0) then
            internal_sdat <= '1'; -- Release the bus so that the slave can acknowledge the address
            bit_count <= 7;
            busy <= '0'; -- Signal that we are not busy so that the user can write a new piece of data to the data port
            state <= slave_ack2; -- Go to next state
            slave_ack <= '1';
          else
            bit_count <= bit_count - 1; -- Decrease the bit counter
            internal_sdat <= internal_data(bit_count - 1); -- Continue writing the data
            state <= data_write; -- Stay in the same state
          end if;

        -- Slave ack 2
        when slave_ack2 =>
          -- Check if the user wants to continue sending new data
          if (enable = '1') then
            busy <= '1'; -- Indicate that we are busy again
            internal_data <= data; -- Get the new data byte
            internal_sdat <= internal_data(bit_count); -- Write the first bit
            state <= data_write; -- Go to the data write state again
          -- Otherwise we should issue a stop command
          else
            busy <= '1'; -- Indicate that we are busy
            enable_sclk <= '0'; -- Disable the clock
            state <= stop; -- Go to the stop command state
          end if;
          slave_ack <= '0';

        -- Stop state
        when stop =>
          slave_ack <= '0';
          state <= ready; -- Go to the ready state to wait for a new transmission

        end case;
    end if;
  end process state_machine;

  -- Set the outpu to of i2c_sdat
  with state select
    sda_enable_n <= state_clk WHEN start,       -- Start condition
                    not state_clk WHEN stop,    -- Stop condition
                    internal_sdat WHEN OTHERS;  -- Otherwise use the internal data signal   
  
  -- Only pull the clock low if it's enabled and the internal clock is low
  i2c_sclk <= '0' when (enable_sclk = '1' and internal_sclk = '0') else 'Z';
  i2c_sdat <= '0' when (sda_enable_n = '0') else 'Z';
  
end architecture behav;

