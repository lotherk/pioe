#!/usr/bin/env ruby
require 'yaml'
require 'erb'

pkgs = ARGV.delete('--with-packages')

config = YAML.load(File.read("config.yaml"))
errors = []
config.each do |family, cfg|
	run = cfg["run"]
	cfg["from"].each do |from|
		image_name = "pioe-" + from.gsub(':', '-')
		container_name =" pioe-build-" + from.gsub(':', '-')
		cmd_rm = "docker rm " + container_name
		cmd_run_add = "-v tmp:/tmp/pkg"
		cmd_run = "docker run --rm " + cmd_run_add + " -t --name " + container_name + " " + image_name
		puts cmd_rm
		puts system(cmd_rm)
		puts cmd_run
		unless system(cmd_run)
			errors << from
		end
	end
end

errors.each do |err|
	$stderr.puts "Failed on " + err
end

exit -1 if errors.count > 0
exit 0
