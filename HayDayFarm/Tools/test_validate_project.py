"""Engine-free regressions for the project/engine include boundary."""

import unittest

import validate_project as validator


class IncludeValidationTests(unittest.TestCase):
    def setUp(self):
        validator.errors.clear()
        self.header = validator.MODULE_ROOT / "Camera/FarmCameraMode_Bench.h"

    def test_missing_project_header_is_rejected(self):
        validator.check_includes(self.header, '#include "Camera/FarmMissingMode.h"')
        self.assertEqual(len(validator.errors), 1)
        self.assertIn("does not exist", validator.errors[0])

    def test_existing_project_header_at_wrong_path_is_rejected(self):
        validator.check_includes(self.header, '#include "Wrong/FarmCameraMode.h"')
        self.assertEqual(len(validator.errors), 1)
        self.assertIn("is wrong", validator.errors[0])

    def test_engine_header_in_shared_directory_is_allowed(self):
        validator.check_includes(self.header, '#include "Camera/CameraComponent.h"')
        self.assertEqual(validator.errors, [])

    def test_existing_and_generated_headers_are_allowed(self):
        validator.check_includes(self.header, '\n'.join([
            '#include "Camera/FarmCameraMode.h"',
            '#include "FarmCameraMode_Bench.generated.h"',
        ]))
        self.assertEqual(validator.errors, [])


if __name__ == "__main__":
    unittest.main()
