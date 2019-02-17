#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum Edge {
    Falling,
    Rising,
    Flat,
}

impl Edge {
    pub fn falling(self) -> bool {
        self == Edge::Falling
    }
    pub fn rising(self) -> bool {
        self == Edge::Rising
    }
    pub fn flat(self) -> bool {
        self == Edge::Flat
    }

    pub fn sig_send(self, mut f: impl FnMut(bool) -> ()) {
        match self {
            Edge::Rising => f(true),
            Edge::Falling => f(false),
            Edge::Flat => (),
        };
    }

    pub fn sig_send_val<T>(self, rise: T, fall: T, mut f: impl FnMut(T) -> ()) {
        match self {
            Edge::Rising => f(rise),
            Edge::Falling => f(fall),
            Edge::Flat => (),
        };
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
        let edge = if self.last_value && !new_value {
            Edge::Falling
        } else if !self.last_value && new_value {
            Edge::Rising
        } else {
            Edge::Flat
        };
        self.last_value = new_value;
        edge
    }

    pub fn falling(&mut self, new_value: bool) -> bool {
        self.get(new_value) == Edge::Falling
    }

    pub fn rising(&mut self, new_value: bool) -> bool {
        self.get(new_value) == Edge::Rising
    }

    pub fn flat(&mut self, new_value: bool) -> bool {
        self.get(new_value) == Edge::Flat
    }
}
