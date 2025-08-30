

use bitfield::bitfield;

#[repr(C,packed)]
struct Packet<const T: usize>{
	pub pid_unk_sized: PidUnkSized,
	pub length: u8,
	pub data: [u8;T]
}
bitfield! {
	pub struct PidUnkSized(u8);
	u8;
	pub pid,   set_pid: 5, 0;
	pub unk,   set_unk: 6, 6;
	pub sized, set_sized:7,7;
	impl Debug;
}

#[repr(C,packed)]
union Report<const T: usize> {
	something: a<T>
}
#[derive(Clone, Copy)]
#[repr(C,packed)]
struct a<const T: usize>{
		pub tagseq:	u8,
		pub data: [u8;T]
}
bitfield! {
	pub struct TagSeq(u8);
	u8;
	pub tag,   set_tag: 3, 0;
	pub seq,   set_seq: 7, 4;
	impl Debug;
}