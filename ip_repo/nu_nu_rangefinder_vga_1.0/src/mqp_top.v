//Copyright 1986-2016 Xilinx, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2016.2 (win64) Build 1577090 Thu Jun  2 16:32:40 MDT 2016
//Date        : Tue Nov 01 20:42:35 2016
//Host        : JOHN-HP running 64-bit major release  (build 7600)
//Command     : generate_target design_1_wrapper.bd
//Design      : design_1_wrapper
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module mqp_top
(
    //physical pins
    input fpga_clk,
    input reset,
    output hsync,
    output vsync,
    output reg [11:0] rgb,
    
    //processing system
    input [27:0] data_enable_step,
    output transmit,
    
    //rangefinder BRAM
    output [7:0] addra1,
    input [12:0] coord1_data,
    output clk_100M1,
    output [7:0] addra2,
    input [12:0] coord2_data,
    output clk_100M2,
    
    //vga map BRAM
    output [8:0] ylocation,
    output clk_100M3,
    output reg [639:0] dina,
    output ena,
    output reg wea,
    
    output [8:0] vcount_9b,
    output clk_25M1,
    input [639:0] x_vga,
    output enb
);
    
    wire clk_100M;   
    wire clk_25M;
    
    //clocks to BRAM
    assign clk_100M1 = clk_100M;
    assign clk_100M2 = clk_100M;
    assign clk_100M3 = clk_100M;
    assign clk_25M1 = clk_25M;
    
    //enables for BRAM
    assign ena = 1'b1;
    assign enb = 1'b1;
    
    //splitting up data, enable, and step data
    assign data = data_enable_step[27:12];
    assign enable = data_enable_step[11];
    assign step = data_enable_step[10:0];
     
    //for rangefinder logic
    wire [15:0] data;
    wire enable;
    wire [10:0] step;
    wire [8:0] xlocation;
     
    // for vga logic
    wire [10:0] hcount, vcount;    // horizontal, vertical location on screen
    assign vcount_9b = vcount[8:0];   // this could cause problems
    wire blank;
    reg [1:0] vga_count;

    
    //writing location to vga BRAM
    //look into this logic*****************  
    always @ (posedge clk_100M)
    begin
        if(write)
        begin
            dina = dina || (5'b11111 << (638 - xlocation));
            wea = 1'b1;
        end
        else
            wea = 1'b0;
    end
       
    //rgb vga logic                  
    always @ (hcount, vcount, blank)
    begin
        if(blank)
            rgb = 12'h000;
        else if(x_vga[hcount] == 1'b1 && vga_count == 2)
            rgb = 12'hF0F;
        else
            rgb = 12'h088;
    end
    
    // delays vga logic by two 100M clock cycles
    // for the BRAM read delay    
    always @ (posedge clk_100M)
    begin
        if(reset || blank)
            vga_count <= 2'b00;
        else if(!blank && vga_count < 2)
            vga_count <= vga_count + 1'b1;
    end
   
   rangefinder rangefinder
   (
       .clk(clk_100M),
       .reset(reset),
       .data(data),
       .enable(enable),
       .step(step),
       .xlocation(xlocation),
       .ylocation(ylocation),
       .write(write),
       .transmit(transmit),
       
       .addra1(addra1),
       .coord1_data(coord1_data),
       
       .addra2(addra2),
       .coord2_data(coord2_data)
   );        
       
   vga_controller_640_60 vga_controller
   (
       .rst(reset),
       .pixel_clk(clk_25M),
       .HS(hsync),
       .VS(vsync),
       .hcount(hcount),
       .vcount(vcount),
       .blank(blank)
   );        
       
   clk_wiz_0 mmcm
   (
       .clk_in1(fpga_clk),
       .clk_100M(clk_100M),
       .clk_25M(clk_25M),
       .reset(reset),
       .locked()
   );       
       
endmodule
