use std::collections::HashMap;
// TODO use FNVhash
use crate::sync::AosMutex;
use std::any::*;

mod imp;
pub use imp::QueueClient as QClient;

/// Marker trait for all Types going into Queues
pub trait QType: Default + Copy + Send + Sync + 'static {}

#[derive(Debug, Copy, Clone, Hash, PartialEq, Eq)]
pub struct QKey(&'static str);

impl QKey {
    pub fn new(name: &'static str) -> Self {
        Self(name)
    }
}

#[derive(Debug)]
pub struct QRegistry {
    table: HashMap<(QKey, TypeId), Box<dyn Any + Send + Sync + 'static>>,
}

impl QRegistry {
    pub fn new() -> AosMutex<Self> {
        AosMutex::new(Self {
            table: HashMap::new(),
        })
    }

    pub fn get_or_insert<T: QType>(&mut self, key: QKey, insert_size: usize) -> QClient<T> {
        let table_key = (key, TypeId::of::<T>());
        let any_casted = self
            .table
            .entry(table_key)
            .or_insert_with(|| Box::new(QClient::<T>::new_queue(insert_size)));
        let boxed = any_casted
            .downcast_ref::<QClient<T>>()
            .expect("Invariants broken in QRegistry!");
        let mut queue = (*boxed).clone();
        // don't get old data, and try to guarantee some good reads
        queue.forget();
        queue
    }
}

lazy_static::lazy_static! {
    static ref GLOBAL_QREGISTRY: AosMutex<QRegistry> = QRegistry::new();
}

pub fn get<T: QType>(key: QKey, insert_size: usize) -> QClient<T> {
    GLOBAL_QREGISTRY.lock().get_or_insert(key, insert_size)
}

pub fn get_local<T: QType>(registry: &mut QRegistry, key: QKey, insert_size: usize) -> QClient<T> {
    registry.get_or_insert(key, insert_size)
}

#[cfg(test)]
mod test {
    use super::*;
    use std::thread;

    impl QType for u32 {}

    #[test]
    fn registry_create() {
        let key = QKey::new("registry_create");
        let q1 = get::<u32>(key, 10);
        q1.push(1234);
        q1.push(1235);
        q1.push(1236);
        thread::spawn(move || {
            // Fail the test if we actually make a new Queue due to impossible size
            let mut q = get::<u32>(key, std::usize::MAX);
            assert_eq!(q.next(), None);
            assert_eq!(q.latest(), 1236);
        })
        .join()
        .unwrap();
    }
}
