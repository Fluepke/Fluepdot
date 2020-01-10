"""Provides utilities"""
import time

def wait_for(predicate: callable, timeout: float, poll_interval: float, *args, **kwargs) -> bool:
    """Wait for the predicate to return true"""
    end_time = time.time() + timeout
    while time.time() < end_time:
        time.sleep(poll_interval)
        if predicate(*args, **kwargs):
            return True
    return False
