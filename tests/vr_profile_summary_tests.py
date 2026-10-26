import importlib.util
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location("summary", Path(__file__).resolve().parents[1] / "tools/summarize-vr-profile.py")
summary = importlib.util.module_from_spec(spec)
spec.loader.exec_module(summary)


class ProfileSummaryTests(unittest.TestCase):
    def test_percentile(self):
        self.assertEqual(summary.percentile([10, 0, 20], .5), 10)
        self.assertEqual(summary.percentile([8], .95), 8)

    def test_window_invalid_samples_and_separate_components(self):
        with tempfile.TemporaryDirectory() as name:
            path = Path(name)
            header = "frame,stage,start_qpc_ms,duration_ms\n"
            (path / "engine.csv").write_text(header + "1,simulation_cpu,1000,50\n2,simulation_cpu,2000,3\npartial")
            (path / "bridge.csv").write_text(header + "4,gpu_frame_elapsed,2001,5\n5,gpu_disjoint,2002,-1\n")
            (path / "renderer.csv").write_text(header + "2,eye0.geometry_primary,2003,2\n")
            text = summary.summarize(path, skip=1, seconds=.5)
            self.assertIn("simulation_cpu", text)
            self.assertIn("3.000", text)
            self.assertNotIn("50.000", text)
            self.assertIn("gpu_disjoint: 1 unavailable samples", text)
            self.assertIn("gpu_frame_elapsed", text)
            self.assertIn("renderer / eye0.geometry_primary", text)


if __name__ == "__main__":
    unittest.main()
