library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- Entity
entity spi_master_controller is
	generic(
		DATA_WIDTH 		: integer := 16;		-- Amount of data for each transfer
		CLK_PRESCALER	: integer := 32);		-- Divide clk with this to get spi_sclk
		-- TODO: Clock polarity, etc...
		-- NOW: CPOL = 1, CPHA = 1
	port(
		clk 				: in std_logic;
		reset_n 			: in std_logic;
		
		-- Module interface
		data_to_send	: in std_logic_vector(DATA_WIDTH-1 downto 0);
		start_transfer	: in std_logic;
		valid_data		: out std_logic;
		data_received	: out std_logic_vector(DATA_WIDTH-1 downto 0);
		busy_transfer	: out std_logic;
		
		-- External hardware interface
		spi_data_in		: in std_logic;
		spi_cs_n			: out std_logic;
		spi_sclk			: out std_logic;
		spi_data_out	: out std_logic);
end spi_master_controller;

architecture behav of spi_master_controller is
	signal clk_counter 				: integer := 0;
	signal bit_counter				: integer := DATA_WIDTH;
	signal stored_data_to_send 	: std_logic_vector(DATA_WIDTH-1 downto 0);
	signal temp_spi_sclk				: std_logic;
	signal synced_data_in			: std_logic;
	signal last_start_transfer		: std_logic;
	signal transfer_is_active		: std_logic;
	signal cs_wait_period			: std_logic;
begin
	process(clk, reset_n)
	begin
		-- Asynchronous reset
		if (reset_n = '0') then
			busy_transfer <= '0';
			data_received <= (others => '0');
			valid_data <= '0';
			
			spi_data_out <= '0';
			spi_cs_n <= '1';
			
			clk_counter <= 0;
			bit_counter <= DATA_WIDTH;
			stored_data_to_send <= (others => '0');
			temp_spi_sclk <= '1';
			synced_data_in <= '0';
			last_start_transfer <= '0';
			transfer_is_active <= '0';
			cs_wait_period <= '0';
		
		-- Synchronous part
		elsif rising_edge(clk) then
			-- Synchronize the data_in signal to our internal clock and use this instead
			synced_data_in <= spi_data_in;
			
			last_start_transfer <= start_transfer;
			-- ====================================================================================
			-- Check for rising edge of start_transfer as that indicates data transfer should start
			-- ====================================================================================
			if (transfer_is_active = '0' and last_start_transfer = '0' and start_transfer = '1') then
				stored_data_to_send <= data_to_send;
				busy_transfer <= '1';
				data_received <= (others => '0');
				valid_data <= '0';
				transfer_is_active <= '1';
				
				spi_cs_n <= '0';
				temp_spi_sclk <= not temp_spi_sclk;
				-- Output the first bit, MSB first
				spi_data_out <= data_to_send(bit_counter - 1);
				bit_counter <= bit_counter - 1;
			-- ====================================================================================
			-- Check if transfer is happening and do things accordingly
			-- ====================================================================================
			elsif (transfer_is_active = '1') then
				-- When we reached the end of half the clock cycle we should toggle the spi clock
				if (clk_counter = CLK_PRESCALER/2 - 1) then
					clk_counter <= 0;
					temp_spi_sclk <= not temp_spi_sclk;
						
					-- On the falling edge we should update the output data value
					if (temp_spi_sclk = '1') then						
						-- Check if it's the last bit
						if (bit_counter = 0) then
							bit_counter <= DATA_WIDTH;
							temp_spi_sclk <= '1';
							transfer_is_active <= '0';
							cs_wait_period <= '1';
							spi_cs_n <= '1';
						else
							spi_data_out <= stored_data_to_send(bit_counter - 1);
							bit_counter <= bit_counter - 1;
						end if;
					-- On falling edge we should read the incoming bit
					elsif (temp_spi_sclk = '0') then
						data_received(bit_counter) <= synced_data_in;
					end if; -- (temp_spi_sclk = '1')
				else
					clk_counter <= clk_counter + 1;
				end if; -- (clk_counter = CLK_PRESCALER-1)
			-- ====================================================================================
			-- Wait half sclk period after cs is set low to allow it to be registred
			-- ====================================================================================
			elsif (cs_wait_period = '1') then
				-- When half a period has gone by we can indicate the transfer is done
				if (clk_counter = CLK_PRESCALER/2 - 1) then
					clk_counter <= 0;
					cs_wait_period <= '0';
					busy_transfer <= '0';
					valid_data <= '1';
				else
					clk_counter <= clk_counter + 1;
				end if;
			
			end if; -- (transfer_is_active = '0' and last_start_transfer = '0' and start_transfer = '1')
	
		end if; -- if (reset_n = '0')
	end process;
	
	spi_sclk <= temp_spi_sclk;

end architecture behav;