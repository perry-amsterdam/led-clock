// Function to create led clock electronics box.
module box(x,y,z,d) {

	// Box for clock electronics.
	difference()
	{
		cube([x, y ,z], center=true);
		translate([0,0,1*d])cube([x-(2*d), y-(2*d) ,z-2*d], center=true);
	}

	// Text on the side of the box.
	translate([0,-y/2,0])rotate([90,0,0])linear_extrude(height=3)text("Led clock electronics !!!", font="tohoma;styles=bold", halign="center", valign="center");

//
//	// Box lid.
//	difference()
//	{
//		union()
//		{
//			translate([0,50+25,0])cube([x, y ,d]);
//			translate([0+d,50+25+d,d])cube([(x - 2*d), (y-(2*d)) ,d]);
//		}
//
//		for(repeat = [0, 50,100,150])
//		{
//			for(count = [0,5,10,15,20])
//			{
//				translate([repeat + 10,60+25+count + d,0])cube([30, 2*d/4 ,2*d]);
//			}
//		}
//		
//		translate([d+d,50+25+d+d,d])cube([(x - 4*d), (y-(4*d)) ,d]);
//	}
//
	// Box dividers.
	for(count = [50,100,150])
	{
//		difference()
//		{	
			translate([-x/2 + d + count,0,0])cube([d,y,z], center=true);
//			translate([count,d,(z-2*d-d*1)])cube([d,2*d,2*d]);
//		}
	}
}

// Call to function, to create the box.
box(200,50,30,3);

