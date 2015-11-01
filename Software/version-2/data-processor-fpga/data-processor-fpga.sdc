create_clock -period 20ns -name clk_50M_1 [get_ports clk_50M_1]
create_clock -period 20ns -name clk_50M_2 [get_ports clk_50M_2]
create_clock -period 20ns -name SPI_DATA_SCK_P [get_ports SPI_DATA_SCK_P]

derive_pll_clocks

derive_clock_uncertainty

derive_clocks -period 10ns