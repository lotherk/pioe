#!/usr/bin/env ruby
require 'yaml'
require 'erb'


config = YAML.load(File.read("config.yaml"))
arg_family = {}
ARGV.each do |argv|
	next if argv =~ /^\-/
	m = argv.match(/(\w+)-(.*)/)
	arg_family[m[1]] ||= []
	arg_family[m[1]] << m[2]
end
threads = []
config.each do |family, cfg|
	run = cfg["run"]
	froms = arg_family[family] || []
	froms << cfg["from"]
	froms.flatten.each do |from|
		t = Thread.new {
			puts from
			erb = ERB.new(File.read("Dockerfile.erb"), nil, "-")
			s = erb.result(binding)
			image_name = "pioe-" + from.gsub(':', '-').gsub('/', '-')
			docker_file = ".dockerfile." + image_name
			File.open(docker_file, "w") do |f|
				f.write(s)
			end
			puts s
			cmd_rmi = "docker rmi -f " + image_name
			cmd_build = "docker build -t " + image_name + " -f " + docker_file + " ."
			puts cmd_rmi
			puts cmd_build

			unless ARGV.delete('--noop')
#				system(cmd_rmi) unless ARGV.delete('--keep')
				system(cmd_build)
			end
			File.unlink(docker_file)
		}
		threads << t
		t.join unless ARGV.delete('--multi')
	end
end

threads.each do |t|
	t.join
end
