mthick = 3;

difference() {
color([0,0,1,0.5]) cube([39, 24, 3]);
translate([15,24-9,0]) cube([9,9,3]);
};

translate([-3,-3,-3]) difference() {
color([0,0,1,0.5]) cube([39+2*3, 24+3, 3]);
translate([19,27-7,0]) cube([7,7,3]);
};

// middle

translate([0,24,-25]) difference() {
color([0,0,1,0.5]) cube([39,3,50]);
translate([15,0,0]) cube([9,3,5]);
    // not to scale
rotate([90,0,0]) translate([12,35,0]) cylinder(3, 2.25/2, 2.25/2, 0);
    // not to scale
rotate([90,0,0]) translate([27,35,0]) cylinder(3, 2.25/2, 2.25/2, 0);
    // not to scale
    translate([18,0,28]) cube([3,3,5]);
}

// side 1

translate([-3,-3,-28]) union() {
color([0,0,1,0.5]) cube([3,24+3,25]);
translate([0,24+3,0]) color([0,0,1,0.5]) cube([3,30,53]);
};

// side 2

translate([39,-3,-28]) union() {
color([0,0,1,0.5]) cube([3,24+3,25]);
translate([0,24+3,0]) color([0,0,1,0.5]) cube([3,30,53]);
};

// bottom
translate([0,-3,-28]) color([0,0,1,0.5]) cube([39,57,3]);

// front panel
translate([0,-3,-25]) color([0,0,1,0.5]) cube([39,3,22]);

// top panel
translate([-3,24,25]) color([0,0,1,0.5]) cube([45,30,3]);

// back panel

translate([0,51,-25])
difference() {
     color([0,0,1,0.5]) cube([39,3,50]);
     translate([15,0,0]) cube([9,3,5]);
}

// lid
translate([0,-13,25]) rotate([-45,0,0]) {
// top
color([0,0,1,0.5]) translate([-3,-3,25]) cube([45,27,3]);

// sides
color([0,0,1,0.5]) translate([-3,-3,0]) cube([3,27,25]);
color([0,0,1,0.5]) translate([39,-3,0]) cube([3,27,25]);

// front
color([0,0,1,0.5]) translate([0,-3,0]) cube([39,3,25]);
};