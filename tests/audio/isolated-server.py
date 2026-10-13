#!/usr/bin/env python3
"""Run a private PipeWire server; never connects to desktop audio. Pass audio-tests binary."""
import os,subprocess,tempfile,time,json,sys,signal
with tempfile.TemporaryDirectory(prefix='caster-private-audio-') as runtime:
 os.chmod(runtime,0o700)
 env=dict(os.environ,XDG_CONFIG_HOME=runtime+'/config',XDG_STATE_HOME=runtime+'/state',PIPEWIRE_REMOTE='pipewire-0',XDG_RUNTIME_DIR=runtime,PIPEWIRE_RUNTIME_DIR=runtime,PULSE_SERVER='unix:'+runtime+'/pulse/native')
 # Use a private policy manager, with every hardware monitor disabled.
 # The installed WirePlumber 0.4 Lua configuration is copied into the test runtime.
 os.makedirs(runtime+'/config/wireplumber/main.lua.d')
 with open('/usr/share/wireplumber/wireplumber.conf') as source:
  wp_config=source.read().replace('{ name = bluetooth.lua, type = config/lua }','')
 with open(runtime+'/config/wireplumber/wireplumber.conf','w') as config:
  config.write(wp_config)
 with open(runtime+'/config/wireplumber/main.lua.d/89-test-no-hardware.lua','w') as config:
  config.write('alsa_monitor.enabled = false\nv4l2_monitor.enabled = false\nlibcamera_monitor.enabled = false\n')
 processes=[]
 def call(*args):
  return subprocess.check_output(args,env=env,stderr=subprocess.STDOUT,text=True).strip()
 try:
  with open(runtime+'/server.log','w') as log:
   processes.append(subprocess.Popen(['pipewire'],env=env,stdout=log,stderr=log,start_new_session=True))
   time.sleep(.5)
   processes.append(subprocess.Popen(['pipewire-pulse'],env=env,stdout=log,stderr=log,start_new_session=True))
   for n in range(40):
    if os.path.exists(runtime+'/pulse/native'):break
    time.sleep(.1)
   assert json.loads(call('pactl','-f','json','info'))['server_string'].startswith('unix:'+runtime)
   processes.append(subprocess.Popen(['dbus-run-session','--','wireplumber'],env=env,stdout=log,stderr=log,start_new_session=True))
   time.sleep(.8)
   args=['sink_name=CasterIsolatedTest','sink_properties="device.description=Caster_Test caster.owner=isolated-test"']
   mid=call('pactl','load-module','module-null-sink',*args)
   modules=json.loads(call('pactl','-f','json','list','modules'))
   module=next(m for m in modules if m['name']=='module-null-sink' and 'CasterIsolatedTest' in m['argument'])
   assert module['argument']==' '.join(args),(module,args)
   sink=next(s for s in json.loads(call('pactl','-f','json','list','sinks')) if s['name']=='CasterIsolatedTest')
   assert sink['properties']['caster.owner']=='isolated-test',sink
   loopargs=['source=CasterIsolatedTest.monitor','sink=CasterIsolatedTest','latency_msec=30','source_dont_move=true','sink_dont_move=true','sink_input_properties="caster.owner=isolated-test"','source_output_properties="caster.owner=isolated-test"']
   loopid=call('pactl','load-module','module-loopback',*loopargs)
   module=next(m for m in json.loads(call('pactl','-f','json','list','modules')) if m['name']=='module-loopback')
   assert module['argument']==' '.join(loopargs),module
   call('pactl','unload-module',loopid);call('pactl','unload-module',mid)
   print('PASS: private PipeWire null-sink ownership properties, exact module arguments, loopback flags, cleanup',flush=True)
   if len(sys.argv)>1:
    subprocess.run([sys.argv[1],'isolatedServerIntegration'],env=dict(env,CASTER_PRIVATE_TEST='1'),check=True,timeout=30)
   if len(sys.argv)>2:
    subprocess.run(sys.argv[2:],env=dict(env,QT_QPA_PLATFORM='offscreen'),check=True,timeout=120)
 finally:
  for p in reversed(processes):
   os.killpg(p.pid,signal.SIGTERM)
   try:p.wait(timeout=3)
   except subprocess.TimeoutExpired:os.killpg(p.pid,signal.SIGKILL);p.wait()
