library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity dip_led_test is
	port(
		CLK_1		: in std_logic;
		CLK_2		: in std_logic;
		DIP_SW0	: in std_logic;
		DIP_SW1 	: in std_logic;
		SPI_NCS	: in std_logic;
		ch0_value : in std_logic_vector(9 downto 0);
		LED_0		: out std_logic;
		LED_1		: out std_logic;
		LED_2		: out std_logic;
		LED_3		: out std_logic;
		LED_4		: out std_logic;
		LED_5		: out std_logic;
		LED_6		: out std_logic;
		LED_7		: out std_logic;
		IO			: out std_logic_vector(41 downto 0);
		RAM		: out std_logic_vector(38 downto 0));
end dip_led_test;

architecture behav of dip_led_test is
	signal internal_led_6 : std_logic := '0';
	signal internal_led_7 : std_logic := '0';
begin
	process(CLK_1)
		variable count : integer range 0 to 50000000;
	begin
		if rising_edge(CLK_1) then
			if count = 25000000 then
				count := 0;
				internal_led_6 <= not internal_led_6;
			else
				count := count + 1;
			end if;
		end if;
	end process;

	process(CLK_2)
		variable count : integer range 0 to 400000000;
	begin
		if rising_edge(CLK_2) then
			if count = 100000000 then
				count := 0;
				internal_led_7 <= not internal_led_7;
			else
				count := count + 1;
			end if;
		end if;
	end process;
		

	LED_7 <= ch0_value(9);
	LED_6 <= ch0_value(8);
	LED_5 <= ch0_value(7);
	LED_4 <= ch0_value(6);
	LED_3 <= ch0_value(5);
	LED_2 <= ch0_value(4);
	LED_1 <= ch0_value(3);
	LED_0 <= ch0_value(2);
--	LED_0 <= DIP_SW0;
--	LED_1 <= DIP_SW1;
--	LED_2 <= SPI_NCS;
--	LED_3 <= not DIP_SW0;
--	LED_4 <= DIP_SW0 and DIP_SW1;
--	LED_5 <= DIP_SW0 or DIP_SW1;
--	LED_6 <= internal_led_6 and DIP_SW0;
--	LED_7 <= internal_led_7 and DIP_SW1;
--	IO(41 downto 1) <= (others => DIP_SW0);
--	IO(0) <= CLK_1;
--	RAM <= (others => DIP_SW0);

end architecture behav;