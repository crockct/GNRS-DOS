
defApplication('root:app:mf_node_mon', 'mf_node_mon') do |app|

  app.version(0, 9, 1)
  app.shortDescription = 'Node resource monitor'
  app.description = %{
	Monitors usage of cpu, memory, disk and nic resources of a physical
        or virtual node, and periodically injects these stats
	for archival at the OML server.
  }
  app.path = "/usr/local/bin/mf_node_mon"

  app.defMeasurement("node_stats") do |mp|
    mp.defMetric("mp_index", "uint32")
    mp.defMetric("node_id", "string")
    mp.defMetric("cpu_usage", "double") 
    mp.defMetric("mem_usage", "double") 
  end

end

# Local Variables:
# mode:ruby
# End:
# vim: ft=ruby:sw=2
