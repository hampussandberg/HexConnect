create_clock -period 20ns -name clk_50M_1 [get_ports clk_50M_1]
create_clock -period 20ns -name clk_50M_2 [get_ports clk_50M_2]

derive_pll_clocks

derive_clocks -period 10ns

derive_clock_uncertainty