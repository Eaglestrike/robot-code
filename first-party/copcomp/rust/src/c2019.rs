#[derive(Debug, Copy, Clone, Serialize, Deserialize)]
pub struct Packet {
    micros: u64,
    x: f32,
    y: f32,
}
