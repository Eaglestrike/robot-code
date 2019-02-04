#[derive(Copy, Clone, PartialEq, Eq)]
pub enum Edge {
    FALLING,
    RISING,
    FLAT,
}

impl Edge {
    pub fn falling(self) -> bool {
        self == Edge::FALLING
    }
    pub fn rising(self) -> bool {
        self == Edge::RISING
    }
    pub fn flat(self) -> bool {
        self == Edge::FLAT
    }
}

#[derive(Clone)]
pub struct EdgeDetector<T: FnMut() -> bool> {
    closure: T,
    last_value: bool,
}

impl<T: FnMut() -> bool> EdgeDetector<T> {
    pub fn new(closure: T) -> Self {
        Self {
            closure,
            last_value: false,
        }
    }

    pub fn get(&mut self) -> Edge {
        let new_value = (self.closure)();
        let last_value = self.last_value;
        self.last_value = new_value;

        if last_value && !new_value {
            Edge::FALLING
        } else if !last_value && new_value {
            Edge::RISING
        } else {
            Edge::FLAT
        }
    }

    pub fn falling(&mut self) -> bool {
        self.get() == Edge::FALLING
    }

    pub fn rising(&mut self) -> bool {
        self.get() == Edge::RISING
    }

    pub fn flat(&mut self) -> bool {
        self.get() == Edge::FLAT
    }
}
