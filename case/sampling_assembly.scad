h_os = 28.1; // height, outside
w_os = 15.1; // width, outside
d_os = 15.1; // depth, outside
material = 3; // 3mm MDF
tube_diam = 6.23; // 0.2ml PCR tube has a maximum diameter of 6.23mm

// inner box

// sensing assembly
color([0,0,1,0.3]) cube([w_os,material,h_os - material]);
translate([0,d_os - material,0]) color([0,0,1,0.3]) cube([w_os,material,h_os - material]);
// LED piece
translate([0,material,0]) color([0,0,1,0.3]) cube([material,d_os - 2*material,h_os - material]);
// sensor piece
translate([w_os - material,material,0]) color([0,0,1,0.3]) cube([material,d_os - 2*material,h_os - material]);
// top piece
difference() {
   translate([0,0,h_os-material]) color([0,0,1,0.3]) cube([w_os, d_os, material]);
   translate([w_os / 2,w_os / 2,h_os-material]) color([0,0,1,0.3]) cylinder(material, tube_diam / 2, tube_diam / 2, 0);
}


