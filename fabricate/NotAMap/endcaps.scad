inch = 25.4;

D = 3 * inch;
H = 1 * inch;

module acr(){
  #translate([9.147,  1.25 * inch, -1])translate([0, -1.5, 0])cube([12.28 * inch, 3, 1*inch + 2]);
  #translate([9.147,  -1.25 * inch, -1])translate([0, -1.5, 0])cube([12.28 * inch, 3, 1*inch + 2]);
}
module endcap(){
  difference(){
    cylinder(h=H, d=D, $fn=100);
    translate([0, 0, -1])cylinder(h=H + 2, d=D-1*inch, $fn=100);
    rotate([0, 0, 45])translate([0, 1.25 * inch, -1])cylinder(h=H+2, d=3, $fn=30);
    rotate([0, 0, 45+90])translate([0, 1.25 * inch, -1])cylinder(h=H+2, d=3, $fn=30);
    translate([-2.5*inch, -3, 2.2])cube([18 * inch, 6, 9 * inch]);
    translate([0, -20, -3])cube([18 * inch, 40, 9 * inch]);
    acr();
  }
}
module endcaps(){
  endcap();
  translate([(16 - 3) * inch, 0, 1*inch])rotate([0, 0, 180])rotate([180, 0, 0])endcap();
}

//endcap();
//endcaps();
//translate([12.28 * inch/2, 1. * inch, -1])cylinder(h=H+2, d=3, $fn=30);
//translate([12.28 * inch/2, -1. * inch, -1])cylinder(h=H+2, d=3, $fn=30);

module support(){
  translate([7.5*inch, 1.25*inch - 1.5 - 5.5, 0])
    difference(){
    linear_extrude(height=inch)polygon([[-3, -4], [-5, 5.5], [5, 5.5],[3, -4]]);
    translate([0, 0, -1])cylinder(d=3, h=inch + 2, $fn=30);
  }
}
module supports(){
  support();
  mirror([0, 1, 0])support();
}
//supports();
//support();
endcap();
