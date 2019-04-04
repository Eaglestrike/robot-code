#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum Edge {
    Falling,
    Rising,
    Flat,
}

impl Edge {
    pub fn sig_send(self, mut f: impl FnMut(bool)) {
        match self {
            Edge::Rising => f(true),
            Edge::Falling => f(false),
            Edge::Flat => (),
        };
    }

    pub fn sig_send_val<T>(self, rise: T, fall: T, mut f: impl FnMut(T)) {
        match self {
            Edge::Rising => f(rise),
            Edge::Falling => f(fall),
            Edge::Flat => (),
        }
    }
}

#[derive(Debug, Clone)]
pub struct EdgeDetector {
    last_value: bool,
}

impl EdgeDetector {
    pub fn new() -> Self {
        Self { last_value: false }
    }

    pub fn get(&mut self, new_value: bool) -> Edge {
        let edge = match (self.last_value, new_value) {
            (true, false) => Edge::Falling,
            (false, true) => Edge::Rising,
            _ => Edge::Flat,
        };
        self.last_value = new_value;
        edge
    }
}
