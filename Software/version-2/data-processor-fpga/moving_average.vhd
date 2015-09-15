-- *******************************************************************************
-- * @file    moving_average.vhd
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
entity moving_average is
  generic(
    NUM_OF_SAMPLES : integer := 8;
    SAMPLE_BIT_SIZE : integer := 10);
  port(
    clk       : in std_logic;
    reset_n     : in std_logic;
    
    sample_0    : in std_logic_vector(SAMPLE_BIT_SIZE-1 downto 0);
    sample_1    : in std_logic_vector(SAMPLE_BIT_SIZE-1 downto 0);
    sample_2    : in std_logic_vector(SAMPLE_BIT_SIZE-1 downto 0);
    sample_3    : in std_logic_vector(SAMPLE_BIT_SIZE-1 downto 0);
    sample_4    : in std_logic_vector(SAMPLE_BIT_SIZE-1 downto 0);
    sample_5    : in std_logic_vector(SAMPLE_BIT_SIZE-1 downto 0);
    sample_6    : in std_logic_vector(SAMPLE_BIT_SIZE-1 downto 0);
    sample_7    : in std_logic_vector(SAMPLE_BIT_SIZE-1 downto 0);
    
    average_0  : out std_logic_vector(SAMPLE_BIT_SIZE-1 downto 0);
    average_1  : out std_logic_vector(SAMPLE_BIT_SIZE-1 downto 0);
    average_2  : out std_logic_vector(SAMPLE_BIT_SIZE-1 downto 0);
    average_3  : out std_logic_vector(SAMPLE_BIT_SIZE-1 downto 0);
    average_4  : out std_logic_vector(SAMPLE_BIT_SIZE-1 downto 0);
    average_5  : out std_logic_vector(SAMPLE_BIT_SIZE-1 downto 0);
    average_6  : out std_logic_vector(SAMPLE_BIT_SIZE-1 downto 0);
    average_7  : out std_logic_vector(SAMPLE_BIT_SIZE-1 downto 0);
    
    new_data    : in std_logic);
end moving_average;

architecture behav of moving_average is
  signal last_new_data : std_logic;
  
  type sample_vector is array(1 to NUM_OF_SAMPLES) of integer;
  signal sample_fifo_0 : sample_vector;
  signal sample_fifo_1 : sample_vector;
  signal sample_fifo_2 : sample_vector;
  signal sample_fifo_3 : sample_vector;
  signal sample_fifo_4 : sample_vector;
  signal sample_fifo_5 : sample_vector;
  signal sample_fifo_6 : sample_vector;
  signal sample_fifo_7 : sample_vector;
  
  signal sum_0 : integer;
  signal sum_1 : integer;
  signal sum_2 : integer;
  signal sum_3 : integer;
  signal sum_4 : integer;
  signal sum_5 : integer;
  signal sum_6 : integer;
  signal sum_7 : integer;
begin
  process(clk, reset_n)
  begin
    -- Asynchronous reset
    if (reset_n = '0') then  
      last_new_data <= '0';
      sample_fifo_0 <= (others => to_integer(unsigned(sample_0)));
      sample_fifo_1 <= (others => to_integer(unsigned(sample_1)));
      sample_fifo_2 <= (others => to_integer(unsigned(sample_2)));
      sample_fifo_3 <= (others => to_integer(unsigned(sample_3)));
      sample_fifo_4 <= (others => to_integer(unsigned(sample_4)));
      sample_fifo_5 <= (others => to_integer(unsigned(sample_5)));
      sample_fifo_6 <= (others => to_integer(unsigned(sample_6)));
      sample_fifo_7 <= (others => to_integer(unsigned(sample_7)));
      
      sum_0 <= NUM_OF_SAMPLES * to_integer(unsigned(sample_0));
      sum_1 <= NUM_OF_SAMPLES * to_integer(unsigned(sample_0));
      sum_2 <= NUM_OF_SAMPLES * to_integer(unsigned(sample_0));
      sum_3 <= NUM_OF_SAMPLES * to_integer(unsigned(sample_0));
      sum_4 <= NUM_OF_SAMPLES * to_integer(unsigned(sample_0));
      sum_5 <= NUM_OF_SAMPLES * to_integer(unsigned(sample_0));
      sum_6 <= NUM_OF_SAMPLES * to_integer(unsigned(sample_0));
      sum_7 <= NUM_OF_SAMPLES * to_integer(unsigned(sample_0));
    
    -- Synchronous part
    elsif rising_edge(clk) then
      last_new_data <= new_data;
      
      -- Check for rising edge of new_data
      if (last_new_data = '0' and new_data = '1') then
        -- Add the new sample to the fifo
        sample_fifo_0 <= to_integer(unsigned(sample_0)) & sample_fifo_0(1 to NUM_OF_SAMPLES - 1);
        sample_fifo_1 <= to_integer(unsigned(sample_0)) & sample_fifo_1(1 to NUM_OF_SAMPLES - 1);
        sample_fifo_2 <= to_integer(unsigned(sample_0)) & sample_fifo_2(1 to NUM_OF_SAMPLES - 1);
        sample_fifo_3 <= to_integer(unsigned(sample_0)) & sample_fifo_3(1 to NUM_OF_SAMPLES - 1);
        sample_fifo_4 <= to_integer(unsigned(sample_0)) & sample_fifo_4(1 to NUM_OF_SAMPLES - 1);
        sample_fifo_5 <= to_integer(unsigned(sample_0)) & sample_fifo_5(1 to NUM_OF_SAMPLES - 1);
        sample_fifo_6 <= to_integer(unsigned(sample_0)) & sample_fifo_6(1 to NUM_OF_SAMPLES - 1);
        sample_fifo_7 <= to_integer(unsigned(sample_0)) & sample_fifo_7(1 to NUM_OF_SAMPLES - 1);
        
        -- Add the new sample to the sum and subtract the oldest sample
        sum_0 <= sum_0 + to_integer(unsigned(sample_0)) - sample_fifo_0(NUM_OF_SAMPLES);
        sum_1 <= sum_1 + to_integer(unsigned(sample_1)) - sample_fifo_1(NUM_OF_SAMPLES);
        sum_2 <= sum_2 + to_integer(unsigned(sample_2)) - sample_fifo_2(NUM_OF_SAMPLES);
        sum_3 <= sum_3 + to_integer(unsigned(sample_3)) - sample_fifo_3(NUM_OF_SAMPLES);
        sum_4 <= sum_4 + to_integer(unsigned(sample_4)) - sample_fifo_4(NUM_OF_SAMPLES);
        sum_5 <= sum_5 + to_integer(unsigned(sample_5)) - sample_fifo_5(NUM_OF_SAMPLES);
        sum_6 <= sum_6 + to_integer(unsigned(sample_6)) - sample_fifo_6(NUM_OF_SAMPLES);
        sum_7 <= sum_7 + to_integer(unsigned(sample_7)) - sample_fifo_7(NUM_OF_SAMPLES);
      end if;
    end if; -- if (reset_n = '0')
  end process;
  
  -- Get the average from the sum
  average_0 <= std_logic_vector(to_unsigned(sum_0 / NUM_OF_SAMPLES, SAMPLE_BIT_SIZE));
  average_1 <= std_logic_vector(to_unsigned(sum_1 / NUM_OF_SAMPLES, SAMPLE_BIT_SIZE));
  average_2 <= std_logic_vector(to_unsigned(sum_2 / NUM_OF_SAMPLES, SAMPLE_BIT_SIZE));
  average_3 <= std_logic_vector(to_unsigned(sum_3 / NUM_OF_SAMPLES, SAMPLE_BIT_SIZE));
  average_4 <= std_logic_vector(to_unsigned(sum_4 / NUM_OF_SAMPLES, SAMPLE_BIT_SIZE));
  average_5 <= std_logic_vector(to_unsigned(sum_5 / NUM_OF_SAMPLES, SAMPLE_BIT_SIZE));
  average_6 <= std_logic_vector(to_unsigned(sum_6 / NUM_OF_SAMPLES, SAMPLE_BIT_SIZE));
  average_7 <= std_logic_vector(to_unsigned(sum_7 / NUM_OF_SAMPLES, SAMPLE_BIT_SIZE));

end architecture behav;