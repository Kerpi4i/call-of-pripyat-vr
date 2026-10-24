"""Decode and validate native X-Ray metadata of all local VR sound excerpts."""
from pathlib import Path
import importlib.util, json, struct, subprocess

ROOT=Path(__file__).resolve().parent.parent
spec=importlib.util.spec_from_file_location('mechanical_sounds',ROOT/'tools/build-vr-mechanical-sounds.py')
audio=importlib.util.module_from_spec(spec);spec.loader.exec_module(audio)
manifest=json.loads((ROOT/'reports/mechanical-sound-cuts.json').read_text())
for entry in manifest:
    path=ROOT/entry['file']
    info=json.loads(subprocess.check_output(['ffprobe','-v','error','-show_entries',
        'stream=channels,sample_rate:format=duration','-of','json',str(path)]))
    assert info['streams'][0]['channels']==1 and info['streams'][0]['sample_rate']=='44100'
    assert abs(float(info['format']['duration'])-(entry['end']-entry['start']))<.025
    for raw in audio.pages(path.read_bytes()):
        page=bytearray(raw);expected=struct.unpack_from('<I',page,22)[0]
        page[22:26]=b'\0'*4;assert audio.crc(page)==expected
    original=ROOT/'reports/archive-inspection/sounds/weapons'/(entry['source']+'.ogg')
    assert audio.comments(path.read_bytes())[0]==audio.comments(original.read_bytes())[0]
    subprocess.run(['ffmpeg','-v','error','-i',str(path),'-f','null','-'],check=True,
        stdout=subprocess.DEVNULL,stderr=subprocess.PIPE)
print(f'PASS: {len(manifest)} mono excerpts, sample rates, durations, Ogg CRCs and native metadata.')
