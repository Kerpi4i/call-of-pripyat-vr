function normal(shader, t_base, t_second, t_detail)
    shader:begin("copvr_scope","copvr_scope"):fog(false):zb(true,false):blend(true,blend.srcalpha,blend.invsrcalpha)
    shader:dx10texture("s_base",t_base)
    shader:dx10texture("s_reticle","wpn\\wpn_crosshair")
    shader:dx10sampler("smp_rtlinear")
    shader:dx10color_write_enable(true,true,true,false)
end
