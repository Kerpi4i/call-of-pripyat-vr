function normal(shader, t_base, t_second, t_detail)
    shader:begin("copvr_zones","copvr_zones")
        :fog(false)
        :zb(false,false)
        :blend(true,blend.srcalpha,blend.invsrcalpha)
        :dx10color_write_enable(true,true,true,false)
end
