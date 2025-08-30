

mod packet;

fn main() {
	println!("Hello, world!");
	output_packet();
}
fn output_packet(){
	let mut suffering = packet::PidUnkSized(0);
	suffering.set_pid(0x11);
	suffering.set_unk(1);
	suffering.set_sized(1);
	let a =  suffering.0;
	let text = format!("{:#x}",a);
	println!("{}",text);
}
