namespace cpp com.nvr.thrift

service nvrWebService
{
    i32 notice(1:string cmd, 2:string jsonObject)
}
