function normal(shader, t_base, t_second, t_detail)
    shader:begin("stub_notransform_t","copvr_pda"):fog(false):zb(false,false):blend(true,blend.srcalpha,blend.invsrcalpha)
    shader:dx10texture("s_base",t_base)
    shader:dx10sampler("smp_rtlinear")
    shader:dx10color_write_enable(true,true,true,true)
end
