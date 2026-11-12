"""Check ES choice values through the real launcher's settings preamble."""
import os
from pathlib import Path
import subprocess
import unittest
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
PORT = ROOT / 'targets/rgdsplus'


class LauncherSettings(unittest.TestCase):
    def launch_settings(self, shader='', inherited='', intensity='', scanlines='', lcd=''):
        script = (PORT / 'launch-rom.sh').read_text()
        script = script.split('case "$rom" in', 1)[0]
        script = script.replace('. /etc/profile.d/001-functions', '''
get_setting() {
    test "$2" = nds && test "$3" = 'Game Name.nds' || exit 99
    case "$1" in
        fun_drastic_shader) printf '%s' "$TEST_SHADER" ;;
        shader) printf '%s' "$TEST_INHERITED" ;;
        fun_drastic_lcd_intensity) printf '%s' "$TEST_INTENSITY" ;;
        fun_drastic_brighten_scanlines) printf '%s' "$TEST_SCANLINES" ;;
        fun_drastic_brighten_lcd) printf '%s' "$TEST_LCD" ;;
        *) exit 98 ;;
    esac
}
realpath() { printf '%s' '/storage/roms/nds/Game Name.nds'; }
''')
        script += '\nprintf "%s|%s|%s|%s" "$FUN_DRASTIC_SHADER" "$FUN_DRASTIC_LCD_INTENSITY" "$FUN_DRASTIC_BRIGHTEN_SCANLINES" "$FUN_DRASTIC_BRIGHTEN_LCD"\n'
        env = dict(os.environ, TEST_SHADER=shader, TEST_INHERITED=inherited,
                   TEST_INTENSITY=intensity, TEST_SCANLINES=scanlines, TEST_LCD=lcd)
        return subprocess.check_output(['bash', '-c', script, 'launcher', 'game'],
                                       env=env, text=True)

    def test_es_choices_reach_launcher(self):
        features = ET.parse(PORT / 'es_features_fun_drastic.cfg')
        items = {f.attrib['value']: f for f in features.findall('.//feature')}
        self.assertEqual(set(items), {
            'fun_drastic_shader', 'fun_drastic_lcd_intensity',
            'fun_drastic_brighten_scanlines', 'fun_drastic_brighten_lcd',
        })
        for shader in items['fun_drastic_shader'].findall('choice'):
            for intensity in items['fun_drastic_lcd_intensity'].findall('choice'):
                s, i = shader.attrib['value'], intensity.attrib['value']
                self.assertEqual(self.launch_settings(s, 'lcd3x', i), f'{s}|{i}||')

    def test_defaults_and_legacy_settings(self):
        for value in ['', 'auto', 'invalid']:
            self.assertEqual(self.launch_settings('lcd3x', '', value), 'lcd3x|original||')
        for value in ['', 'auto']:
            self.assertEqual(self.launch_settings(value, 'lcd3x'), 'lcd3x|original||')
        self.assertEqual(self.launch_settings('none', 'lcd3x', 'strong'), 'none|strong||')
        self.assertEqual(self.launch_settings(), '|original||')

    def test_shader_parameters_reach_hook(self):
        self.assertEqual(
            self.launch_settings('retroarch-lcd3x', intensity='strong', scanlines='4.5', lcd='2.1'),
            'retroarch-lcd3x|strong|4.5|2.1',
        )


if __name__ == '__main__':
    unittest.main()
