`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    09:07:11 07/19/2016 
// Design Name: 
// Module Name:    fifo_read 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module fifo_read(
	input reset_pointer,
	input get_data, // from microblaze (sent to trigger new read from FIFO to FPGA buffer)
	input [9:0] pixel_addr, // from microblaze, 0-751
	input [7:0] fifo_data, // 8 bit data in from fifo
	input fifo_rck, // 1MHz clock signal generated by FPGA
	output reg fifo_rrst, // fifo read reset (reset read addr pointer to 0)
	output reg fifo_oe, // fifo output enable (allow for addr pointer to increment)
	output reg buffer_ready,
	output [7:0] pixel_value // 8-bit value from internal buffer
   );

parameter [1:0] ready = 2'b00;
parameter [1:0] read = 2'b01;
parameter [1:0] done = 2'b10;  
reg [1:0] state = ready;

reg [7:0] pixel_line [0:751];
reg [9:0] pixel = 10'b00_0000_0000;

always @(posedge fifo_rck)
begin
		if(reset_pointer) 
			fifo_rrst <= 1'b0;
		else if((state==ready) && (get_data))
			begin
			buffer_ready <= 1'b0;
			state <= read;
			fifo_oe <= 1'b0; // allow for read pointer to increment
			end
		else if((state == read) && (pixel == 10'b00_0000_0000))
			begin
			pixel_line[pixel] <= fifo_data;
			pixel <= pixel + 1'b1;
			end
		else if((state == read) && (pixel < 751))
			begin
			fifo_rrst <= 1'b1;
			pixel_line[pixel] <= fifo_data;
			pixel <= pixel + 1'b1;
			end
		else if((state == read) && (pixel == 751))
			begin
			pixel_line[pixel] <= fifo_data;
			state <= done;
			end
		else // state = done
			begin
			buffer_ready <= 1'b1;
			state <= ready;
			fifo_oe <= 1'b1;
			pixel <= 10'b00_0000_000;
			end
end


assign pixel_value [7:0] = pixel_line[pixel_addr];

endmodule
