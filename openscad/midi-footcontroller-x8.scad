/*
multiple footswitches, newer version than 'multi-footswitchx'
 
4x2

print with PLA to prevent warping

custom own pcb.
*/

$fn=20;


WALL_THICKNESS = 2;
FRONT_THICKNESS = 3;

BACK_THICKNESS = 1.3;
BACK_THICKNESS_OUTER = 1.0;  // minimum 1

MOUNT_X = 5;
MOUNT_Y = 5;
MOUNT_R_OUTER = 4;
MOUNT_R_INNER = 1.5;

FOOTSWITCH_HOLE_R = 6.25;
FOOTSWITCH_NUM_X = 4;
FOOTSWITCH_NUM_Y = 2;
FOOTSWITCH_SPACING_X = 50;
FOOTSWITCH_SPACING_Y = 80;
FOOTSWITCH_OFFSET_X = 15;
FOOTSWITCH_OFFSET_Y = 15;

WIDTH = 2*FOOTSWITCH_OFFSET_X + (FOOTSWITCH_NUM_X-1)*FOOTSWITCH_SPACING_X;  //180;
HEIGHT = 2*FOOTSWITCH_OFFSET_Y + (FOOTSWITCH_NUM_Y-1)*FOOTSWITCH_SPACING_Y;  // actually depth in our context
DEPTH = 32;  // actually height in our context
ROUNDING_R = 3.5;

PSU_R = 4.1;  // a separate PSU plug
JACK_R = 5.8;

echo("WIDTH:", WIDTH);

// back

//BACK_DEPTH = 3;

module rounded_cube(x, y, z, r) {
	translate([r, r, 0])
	minkowski() {
		cylinder(r=r, h=1);
		cube([x - 2*r, y - 2*r, z-1]);
	}
}



module tapped_hole(r1, r2, h1, h2, h3=5) {
   // make some kind of a funnell
	// for use with screws
	// it is on purpose a little bit bigger than advertised. it is used for subtraction

	translate([0, 0, -1])
	cylinder(r=r1, h=h1+2);
	translate([0, 0, h1])
	cylinder(r1=r1, r2=r2, h=h2+0.1);
	translate([0, 0, h1+h2])
	cylinder(r=r2, h=h3+0.1);
}

module mount_tube(x, y, h1, h2) {
	difference() {
		translate([x, y, h1-0.5])
		cylinder(r=MOUNT_R_OUTER, h=h2-1);

		translate([x, y, h1-1])
		cylinder(r=MOUNT_R_INNER, h=h2);
	}

}

module midi_hole() {
	translate([0,0,-1])
	cylinder(r=8, h=10, $fn=20);
	translate([11.25,0,-1])
	cylinder(r=1.7, h=10, $fn=20);
	translate([-11.25,0,-1])
	cylinder(r=1.7, h=10, $fn=20);
}

module psu_hole() {
	// psu hole
	translate([0,0,-0.1])
	//rotate([-90,0,0])
	//translate([0,0,-0.1])
	cylinder(r=PSU_R, h=10);
}

module jack_hole() {
	// expression pedal
	translate([0,0,-0.1])
	#cylinder(r=JACK_R, h=32);
}

module footswitch_hole() {
	translate([0,0,-0.1])
	#cylinder(r=FOOTSWITCH_HOLE_R, h=22);
	*translate([-6,-11,10])
	#cube([12,30,15]);
}

module mounting_tube(r1=3, r2=1, h=10) {
	difference() {
		cylinder(r=r1, h=h);
		translate([0,0,-0.1])
		cylinder(r=r2, h=h+0.2);
	}
}

module arduino_approx() {
	#cube([67, 53, 20]);
}

module arduino_mount(r1, r2, h) {
	translate([2.5, 18, 0])
	mounting_tube(r1=r1, r2=r2, h=h);

	translate([2.5, 46, 0])
	mounting_tube(r1=r1, r2=r2, h=h);

	translate([53, 3, 0])
	mounting_tube(r1=r1, r2=r2, h=h);

	translate([55, 50, 0])
	mounting_tube(r1=r1, r2=r2, h=h);
}

module pcb_mock() {
    #cube([68,68,22]);

    translate([-10,5.5+10.5,12])
    rotate([0,90,0])
    #cylinder(r=9, h=25);

    translate([-10,29.5+10.5,12])
    rotate([0,90,0])
    #cylinder(r=9, h=25);
    
    translate([-10,53,2])
    #cube([25,10,12]);
}
// the pcb is 68x68mm
module pcb_mount(r1, r2, h) {
    translate([3.3, 3.3, 0])
	mounting_tube(r1=r1, r2=r2, h=h);

	translate([3.3, 66, 0])
	mounting_tube(r1=r1, r2=r2, h=h);

	translate([64.5, 47, 0])
	mounting_tube(r1=r1, r2=r2, h=h);
    
}

module led8_holes() {
	for (i=[0:7]) {
		translate([i*6.7,0,-0.1])
		cylinder(r=2.5, h=10);
	}

	translate([13.5-3.5,-4.1,0.3])
	cylinder(r=1.25,h=10);
	translate([40.5-3.5,-4.1,0.3])
	cylinder(r=1.25,h=10);
}

module led8_mount(r1, r2, h) {
	translate([13.5-3.5,-4.1,0])
	mounting_tube(r1=r1,r2=r2,h=h);
	translate([40.5-3.5,-4.1,0])
	mounting_tube(r1=r1,r2=r2,h=h);
}

module base() {
union() {
	difference() {
		rounded_cube(WIDTH, HEIGHT, DEPTH, ROUNDING_R);

		translate([WALL_THICKNESS, WALL_THICKNESS, FRONT_THICKNESS])
		rounded_cube(
			WIDTH-2*WALL_THICKNESS, HEIGHT-2*WALL_THICKNESS, 
			DEPTH, ROUNDING_R-WALL_THICKNESS);

		for (y=[0:FOOTSWITCH_NUM_Y-1]) {
		for (x=[0:FOOTSWITCH_NUM_X-1]) {
			translate([FOOTSWITCH_OFFSET_X+x*FOOTSWITCH_SPACING_X, FOOTSWITCH_OFFSET_Y+y*FOOTSWITCH_SPACING_Y, -1])
			footswitch_hole();
		}
		}

		*translate([0, 40, 14])
		rotate([90,0,0])
		rotate([0,90,0])
		#midi_hole();

		*translate([0, 40+30, 14])
		rotate([90,0,0])
		rotate([0,90,0])
		#midi_hole();

		*translate([WIDTH/2, HEIGHT, 14])
		rotate([90,0,0])
		psu_hole();

		translate([WIDTH, 40, 14])
		rotate([0,-90,0])
		jack_hole();

		translate([WIDTH, 40+30, 14])
		rotate([0,-90,0])
		jack_hole();

		*translate([55, 28, 5])
		arduino_approx();

		translate([33, 73, 0])
		led8_holes();
	}

	mount_tube(MOUNT_X, MOUNT_Y, FRONT_THICKNESS, DEPTH-FRONT_THICKNESS);
	mount_tube(WIDTH-MOUNT_X, MOUNT_Y, FRONT_THICKNESS, DEPTH-FRONT_THICKNESS);
	mount_tube(MOUNT_X, HEIGHT-MOUNT_Y, FRONT_THICKNESS, DEPTH-FRONT_THICKNESS);
	mount_tube(WIDTH-MOUNT_X, HEIGHT-MOUNT_Y, FRONT_THICKNESS, DEPTH-FRONT_THICKNESS);
	mount_tube(WIDTH/2, MOUNT_Y, FRONT_THICKNESS, DEPTH-FRONT_THICKNESS);
	mount_tube(WIDTH/2, HEIGHT-MOUNT_Y, FRONT_THICKNESS, DEPTH-FRONT_THICKNESS);


	// stronger chassis
	translate([WIDTH/2-WALL_THICKNESS/2,1,1])
	cube([WALL_THICKNESS, HEIGHT-2, DEPTH-10]);

	*translate([135,1,1])
	cube([WALL_THICKNESS, HEIGHT-2, DEPTH-10]);

    translate([33, 73, 0.1])
    led8_mount(3, 1.25, 3.5);

}

}

module base_back() {
	difference() {
		union() {
			translate([0,0, BACK_THICKNESS_OUTER-0.1])
			rounded_cube(WIDTH, HEIGHT, BACK_THICKNESS+0.1, ROUNDING_R);

			translate([WALL_THICKNESS+0.5, WALL_THICKNESS+0.5, 0])
			rounded_cube(WIDTH-2*WALL_THICKNESS-2*0.5, HEIGHT-2*WALL_THICKNESS-2*0.5, BACK_THICKNESS_OUTER+0.1, 2);

		}

		translate([MOUNT_X, MOUNT_Y, 0])
		tapped_hole(r1=2, r2=4, h1=1+BACK_THICKNESS-BACK_THICKNESS_OUTER, h2=2);
		translate([WIDTH-MOUNT_X, MOUNT_Y, 0])
		tapped_hole(r1=2, r2=4, h1=1+BACK_THICKNESS-BACK_THICKNESS_OUTER, h2=2);
		translate([MOUNT_X, HEIGHT-MOUNT_Y, 0])
		tapped_hole(r1=2, r2=4, h1=1+BACK_THICKNESS-BACK_THICKNESS_OUTER, h2=2);
		translate([WIDTH-MOUNT_X, HEIGHT-MOUNT_Y, 0])
		tapped_hole(r1=2, r2=4, h1=1+BACK_THICKNESS-BACK_THICKNESS_OUTER, h2=2);

		translate([WIDTH/2, MOUNT_Y, 0])
		tapped_hole(r1=2, r2=4, h1=1+BACK_THICKNESS-BACK_THICKNESS_OUTER, h2=2);
		translate([WIDTH/2, HEIGHT-MOUNT_Y, 0])
		tapped_hole(r1=2, r2=4, h1=1+BACK_THICKNESS-BACK_THICKNESS_OUTER, h2=2);
	}
}

module back() {
mirror([0,0,1])
//rotate([180,0,0])
base_back();
translate([WALL_THICKNESS+1.5,22,-0.1])
pcb_mount(4, 1.5, 6);

*translate([WALL_THICKNESS+0.5,22,6])
pcb_mock();
}

*difference() {
base();
translate([0, HEIGHT, DEPTH-BACK_THICKNESS+BACK_THICKNESS_OUTER])
rotate([0,180,0])
rotate([0,0,180])
translate([WALL_THICKNESS+0.5,22,6])
pcb_mock();
}

//translate([0, HEIGHT, DEPTH-BACK_THICKNESS+BACK_THICKNESS_OUTER])
//rotate([0,180,0])
//rotate([0,0,180])
back();